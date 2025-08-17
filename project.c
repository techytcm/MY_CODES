// house_rental_system_with_animated_splash.c
// Cross-Platform House Rental Management System (Console)
// Roles: Admin, Landlord, Tenant
// Files: users.txt, houses.txt, rentals.txt (pipe-delimited)
// Input: defensive fgets + validation (no scanf lockups)
// Splash screen: blinking + gradient + animated reveal

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>   // usleep (POSIX)
#endif

// ---------------- Config ----------------
#define MAX_USERS    1000
#define MAX_HOUSES   2000
#define MAX_RENTALS  4000

// Define constants if not available
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#ifndef DISABLE_NEWLINE_AUTO_RETURN
#define DISABLE_NEWLINE_AUTO_RETURN 0x0008
#endif

// ---------------- ANSI Colors -----------
#define RESET   "\x1b[0m"
#define BLINK   "\x1b[5m"
#define RED     "\x1b[1;31m"
#define GREEN   "\x1b[1;32m"
#define YELLOW  "\x1b[1;33m"
#define BLUE    "\x1b[1;34m"
#define PURPLE  "\x1b[1;35m"
#define CYAN    "\x1b[1;36m"
#define MAGENTA "\x1b[1;95m"

// ---------------- Sleep helpers ---------
static void sleep_ms(unsigned ms){
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 100);
#endif
}

static void sleep_us(unsigned us){
#ifdef _WIN32
    // approximate micro-sleeps with ms
    if(us==0) return;
    Sleep((us+99)/100);
#else
    usleep(us);
#endif
}

// ---------------- Clear + VT mode -------
static void clear_screen(void){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

#ifdef _WIN32
// Enable Virtual Terminal Processing on Windows 10+
static void enable_vt_mode(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    // Enable VT processing
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    // Optionally disable newline auto return
    // dwMode |= DISABLE_NEWLINE_AUTO_RETURN;

    SetConsoleMode(hOut, dwMode);
}

// Alternative version with error checking
static int enable_vt_mode_with_check(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return 0; // Failed to get handle
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return 0; // Failed to get console mode
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(hOut, dwMode)) {
        return 0; // Failed to set console mode
    }

    return 1; // Success
}

#else
static void enable_vt_mode(void) {
    // No-op on non-Windows platforms
}

static int enable_vt_mode_with_check(void) {
    return 1; // Always succeed on non-Windows
}
#endif

// ---------------- Data Types ------------
typedef enum { ROLE_ADMIN=0, ROLE_LANDLORD=1, ROLE_TENANT=2 } UserRole;
typedef enum { STATUS_AVAILABLE=0, STATUS_RENTED=1, STATUS_MAINTENANCE=2 } HouseStatus;

typedef struct {
    int id;
    char username[50];
    char password[50];
    char full_name[100];
    char email[100];
    char phone[20];
    UserRole role;
    bool is_active;
} User;

typedef struct {
    int id;
    char title[100];
    char address[200];
    char city[50];
    char area[50];
    int bedrooms;
    int bathrooms;
    double rent;
    char description[500];
    int landlord_id;
    char landlord_name[100];
    HouseStatus status;
    char date_added[20]; // YYYY-MM-DD
} House;

typedef struct {
    int id;
    int house_id;
    int tenant_id;
    int landlord_id;
    char tenant_name[100];
    char house_title[100];
    char rental_date[20]; // YYYY-MM-DD
    double monthly_rent;
    bool is_active;
} Rental;

// ---------------- Globals ----------------
static User   users[MAX_USERS];     static int user_count=0;
static House  houses[MAX_HOUSES];   static int house_count=0;
static Rental rentals[MAX_RENTALS]; static int rental_count=0;

// ---------------- Utilities --------------
static void trim_newline(char* s){
    if(s) s[strcspn(s,"\r\n")] = 0;
}

static void pause_enter(void){
    printf("\n" YELLOW "Press Enter to continue..." RESET);
    fflush(stdout);
    int c;
    while((c=getchar())!='\n' && c!=EOF) {}
}

static void input_line(const char* prompt, char* buf, size_t n){
    if(!prompt || !buf || n==0) return;
    printf("%s", prompt);
    fflush(stdout);
    if(!fgets(buf,(int)n,stdin)){
        buf[0]='\0';
        clearerr(stdin);
        return;
    }
    trim_newline(buf);
}

static int read_int_range(const char* prompt, int minv, int maxv, int def_if_blank, bool allow_blank){
    char line[64];
    for(;;){
        input_line(prompt, line, sizeof(line));
        if(allow_blank && line[0]=='\0') return def_if_blank;
        char *end;
        long v=strtol(line,&end,10);
        if(end!=line && *end=='\0' && v>=minv && v<=maxv) return (int)v;
        printf(RED "Invalid integer (expected %d..%d). Try again.\n" RESET, minv, maxv);
    }
}

static double read_double_nonneg(const char* prompt, double def_if_blank, bool allow_blank){
    char line[64];
    for(;;){
        input_line(prompt, line, sizeof(line));
        if(allow_blank && line[0]=='\0') return def_if_blank;
        char *end;
        double v=strtod(line,&end);
        if(end!=line && *end=='\0' && v>=0.0) return v;
        printf(RED "Invalid number (>=0). Try again.\n" RESET);
    }
}

static const char* today(void){
    static char d[20];
    time_t t=time(NULL);
    struct tm* lt=localtime(&t);
    if(lt) {
        strftime(d,sizeof(d),"%Y-%m-%d",lt);
    } else {
        strncpy(d,"1970-01-01",sizeof(d)-1);
        d[sizeof(d)-1]='\0';
    }
    return d;
}

static const char* role_str(UserRole r){
    return (r==ROLE_ADMIN)?"Admin":(r==ROLE_LANDLORD)?"Landlord":"Tenant";
}

static const char* status_str(HouseStatus s){
    return (s==STATUS_AVAILABLE)?"Available":(s==STATUS_RENTED)?"Rented":"Maintenance";
}

static int next_user_id(void){
    int mx=0;
    for(int i=0;i<user_count;i++)
        if(users[i].id>mx) mx=users[i].id;
    return mx+1;
}

static int next_house_id(void){
    int mx=0;
    for(int i=0;i<house_count;i++)
        if(houses[i].id>mx) mx=houses[i].id;
    return mx+1;
}

static int next_rental_id(void){
    int mx=0;
    for(int i=0;i<rental_count;i++)
        if(rentals[i].id>mx) mx=rentals[i].id;
    return mx+1;
}

// ---------------- Splash / Menus ----------
// Fancy animated splash: blinking + gradient + reveal
static void type_animated(const char* text, const char* color, unsigned us_per_char){
    if(!text || !color) return;
    printf("%s", color);
    for(size_t i=0; text[i]; ++i){
        putchar(text[i]);
        fflush(stdout);
        sleep_us(us_per_char);
    }
    printf(RESET "\n");
}

static void splash(void){
    clear_screen();
    const char* colors[] = {YELLOW};
    const int ncolors = 1;

    printf(YELLOW "................................\n" RESET);
    sleep_ms(100);

    const char* msg = " WELCOME TO HOUSE RENTAL SYSTEM";
    int cidx = 0;
    for(size_t i=0; msg[i]; ++i){
        if(msg[i]==' '){
            putchar(' ');
        } else {
            printf("%s%s%c" RESET, colors[cidx], BLINK, msg[i]);
            cidx = (cidx + 1) % ncolors;
        }
        fflush(stdout);
        sleep_ms(10);  // 10 ms reveal
    }
    printf("\n\n");
    type_animated("      Developed by TCM", PURPLE, 500);

    printf(YELLOW "................................\n" RESET);
    sleep_ms(1000);

    printf("\n" YELLOW "Press Enter to continue..." RESET);
    fflush(stdout);
    int ch;
    while((ch=getchar())!='\n' && ch!=EOF) {}
}

// ---------------- File I/O -----------------
static void load_users(void){
    FILE* fp=fopen("users.txt","r");
    if(!fp) return;
    char line[1024];
    while(fgets(line,sizeof(line),fp)){
        User u;
        memset(&u, 0, sizeof(u));
        int role, active;
        if(sscanf(line,"%d|%49[^|]|%49[^|]|%99[^|]|%99[^|]|%19[^|]|%d|%d",
                  &u.id,u.username,u.password,u.full_name,u.email,u.phone,&role,&active)==8){
            u.role=(UserRole)role;
            u.is_active=(bool)active;
            if(user_count<MAX_USERS) users[user_count++]=u;
        }
    }
    fclose(fp);
}

static void save_users(void){
    FILE* fp=fopen("users.txt","w");
    if(!fp) return;
    for(int i=0;i<user_count;i++)
        fprintf(fp,"%d|%s|%s|%s|%s|%s|%d|%d\n",
            users[i].id,users[i].username,users[i].password,users[i].full_name,
            users[i].email,users[i].phone,users[i].role,users[i].is_active);
    fclose(fp);
}

static void load_houses(void){
    FILE* fp=fopen("houses.txt","r");
    if(!fp) return;
    char line[2048];
    while(fgets(line,sizeof(line),fp)){
        House h;
        memset(&h, 0, sizeof(h));
        int status;
        if(sscanf(line,"%d|%99[^|]|%199[^|]|%49[^|]|%49[^|]|%d|%d|%lf|%499[^|]|%d|%99[^|]|%d|%19[^\n]",
                  &h.id,h.title,h.address,h.city,h.area,&h.bedrooms,&h.bathrooms,&h.rent,
                  h.description,&h.landlord_id,h.landlord_name,&status,h.date_added)==13){
            h.status=(HouseStatus)status;
            if(house_count<MAX_HOUSES) houses[house_count++]=h;
        }
    }
    fclose(fp);
}

static void save_houses(void){
    FILE* fp=fopen("houses.txt","w");
    if(!fp) return;
    for(int i=0;i<house_count;i++)
        fprintf(fp,"%d|%s|%s|%s|%s|%d|%d|%.2f|%s|%d|%s|%d|%s\n",
            houses[i].id,houses[i].title,houses[i].address,houses[i].city,houses[i].area,
            houses[i].bedrooms,houses[i].bathrooms,houses[i].rent,houses[i].description,
            houses[i].landlord_id,houses[i].landlord_name,houses[i].status,houses[i].date_added);
    fclose(fp);
}

static void load_rentals(void){
    FILE* fp=fopen("rentals.txt","r");
    if(!fp) return;
    char line[1024];
    while(fgets(line,sizeof(line),fp)){
        Rental r;
        memset(&r, 0, sizeof(r));
        int active;
        if(sscanf(line,"%d|%d|%d|%d|%99[^|]|%99[^|]|%19[^|]|%lf|%d",
                  &r.id,&r.house_id,&r.tenant_id,&r.landlord_id,r.tenant_name,
                  r.house_title,r.rental_date,&r.monthly_rent,&active)==9){
            r.is_active=(bool)active;
            if(rental_count<MAX_RENTALS) rentals[rental_count++]=r;
        }
    }
    fclose(fp);
}

static void save_rentals(void){
    FILE* fp=fopen("rentals.txt","w");
    if(!fp) return;
    for(int i=0;i<rental_count;i++)
        fprintf(fp,"%d|%d|%d|%d|%s|%s|%s|%.2f|%d\n",
            rentals[i].id,rentals[i].house_id,rentals[i].tenant_id,rentals[i].landlord_id,
            rentals[i].tenant_name,rentals[i].house_title,rentals[i].rental_date,
            rentals[i].monthly_rent,rentals[i].is_active);
    fclose(fp);
}

// --------------- Find Helpers -------------
static User*  find_user_by_id(int id){
    for(int i=0;i<user_count;i++)
        if(users[i].id==id) return &users[i];
    return NULL;
}

static House* find_house_by_id(int id){
    for(int i=0;i<house_count;i++)
        if(houses[i].id==id) return &houses[i];
    return NULL;
}

static Rental* find_rental_by_id(int id){
    for(int i=0;i<rental_count;i++)
        if(rentals[i].id==id) return &rentals[i];
    return NULL;
}

// --------------- Auth ----------------------
static User* authenticate(void){
    char uname[64], pw[64];
    input_line("Username: ", uname, sizeof(uname));
    input_line("Password: ", pw, sizeof(pw));
    for(int i=0;i<user_count;i++){
        if(strcmp(users[i].username,uname)==0 && strcmp(users[i].password,pw)==0){
            if(!users[i].is_active){
                printf(RED "Account inactive.\n" RESET);
                return NULL;
            }
            return &users[i];
        }
    }
    printf(RED "Invalid credentials.\n" RESET);
    return NULL;
}

static void register_user(void){
    if(user_count>=MAX_USERS){
        printf(RED "User capacity reached.\n" RESET);
        return;
    }
    User u;
    memset(&u,0,sizeof(u));
    u.id = next_user_id();
    input_line("Username: ", u.username, sizeof(u.username));

    // uniqueness check
    for(int i=0;i<user_count;i++){
        if(strcmp(users[i].username,u.username)==0){
            printf(RED "Username already exists.\n" RESET);
            return;
        }
    }

    input_line("Password: ", u.password, sizeof(u.password));
    input_line("Full name: ", u.full_name, sizeof(u.full_name));
    input_line("Email: ", u.email, sizeof(u.email));
    input_line("Phone: ", u.phone, sizeof(u.phone));
    printf("Role: 0=Admin, 1=Landlord, 2=Tenant\n");
    u.role = (UserRole)read_int_range("Select role: ",0,2,2,false);
    u.is_active = true;
    users[user_count++] = u;
    save_users();
    printf(GREEN "Registered user with ID %d\n" RESET, u.id);
}

// --------------- Admin Features ------------
static void admin_list_users(void){
    printf(CYAN "\n-- Users --\n" RESET);
    printf("%-4s | %-14s | %-22s | %-9s | %-6s\n","ID","Username","Full Name","Role","Active");
    for(int i=0;i<user_count;i++){
        printf("%-4d | %-14s | %-22s | %-9s | %-6s\n",
               users[i].id, users[i].username, users[i].full_name,
               role_str(users[i].role), users[i].is_active?"Yes":"No");
    }
}

static void admin_toggle_active(void){
    int id = read_int_range("User ID to toggle active: ",1,2147483647,0,false);
    User* u = find_user_by_id(id);
    if(!u){
        printf(RED "User not found.\n" RESET);
        return;
    }
    u->is_active = !u->is_active;
    save_users();
    printf(GREEN "User %d active=%s\n" RESET, id, u->is_active?"true":"false");
}

static void admin_reset_password(void){
    int id = read_int_range("User ID to reset password: ",1,2147483647,0,false);
    User* u = find_user_by_id(id);
    if(!u){
        printf(RED "User not found.\n" RESET);
        return;
    }
    strncpy(u->password,"1234",sizeof(u->password)-1);
    u->password[sizeof(u->password)-1]='\0';
    save_users();
    printf(GREEN "Password reset to '1234' for user %d\n" RESET, id);
}

static void admin_list_houses(void){
    printf(CYAN "\n-- Houses --\n" RESET);
    printf("%-4s | %-18s | %-10s | %-10s | %3s | %3s | %-12s | %-9s\n",
           "ID","Title","City","Area","Bd","Bt","Status","Rent");
    for(int i=0;i<house_count;i++){
        printf("%-4d | %-18s | %-10s | %-10s | %3d | %3d | %-12s | %9.2f\n",
               houses[i].id, houses[i].title, houses[i].city, houses[i].area,
               houses[i].bedrooms, houses[i].bathrooms, status_str(houses[i].status),
               houses[i].rent);
    }
}

static void admin_list_rentals(void){
    printf(CYAN "\n-- Rentals --\n" RESET);
    printf("%-4s | %-18s | %-18s | %-10s | %-6s | %-9s\n",
           "ID","Tenant","House","StartDate","Active","Rent");
    for(int i=0;i<rental_count;i++){
        printf("%-4d | %-18s | %-18s | %-10s | %-6s | %9.2f\n",
               rentals[i].id, rentals[i].tenant_name, rentals[i].house_title,
               rentals[i].rental_date, rentals[i].is_active?"Yes":"No",
               rentals[i].monthly_rent);
    }
}

// --------------- Landlord Features ---------
static void landlord_list_my_houses(const User* owner){
    printf(CYAN "\n-- My Houses (%s) --\n" RESET, owner->full_name);
    printf("%-4s | %-18s | %-10s | %-10s | %3s | %3s | %-12s | %-9s\n",
           "ID","Title","City","Area","Bd","Bt","Status","Rent");
    for(int i=0;i<house_count;i++){
        if(houses[i].landlord_id==owner->id){
            printf("%-4d | %-18s | %-10s | %-10s | %3d | %3d | %-12s | %9.2f\n",
                   houses[i].id, houses[i].title, houses[i].city, houses[i].area,
                   houses[i].bedrooms, houses[i].bathrooms, status_str(houses[i].status),
                   houses[i].rent);
        }
    }
}

static void landlord_add_house(User* owner){
    if(house_count>=MAX_HOUSES){
        printf(RED "House capacity reached.\n" RESET);
        return;
    }
    House h;
    memset(&h,0,sizeof(h));
    h.id = next_house_id();
    input_line("Title: ", h.title, sizeof(h.title));
    input_line("Address: ", h.address, sizeof(h.address));
    input_line("City: ", h.city, sizeof(h.city));
    input_line("Area: ", h.area, sizeof(h.area));
    h.bedrooms  = read_int_range("Bedrooms (0-50): ",0,50,0,false);
    h.bathrooms = read_int_range("Bathrooms (0-50): ",0,50,0,false);
    h.rent      = read_double_nonneg("Monthly Rent: ", 0, false);
    input_line("Description: ", h.description, sizeof(h.description));
    h.landlord_id = owner->id;
    strncpy(h.landlord_name, owner->full_name, sizeof(h.landlord_name)-1);
    h.landlord_name[sizeof(h.landlord_name)-1] = '\0';
    strncpy(h.date_added, today(), sizeof(h.date_added)-1);
    h.date_added[sizeof(h.date_added)-1] = '\0';
    h.status = STATUS_AVAILABLE;
    houses[house_count++] = h;
    save_houses();
    printf(GREEN "House added with ID %d\n" RESET, h.id);
}

static void landlord_change_status(User* owner){
    int id = read_int_range("House ID to change status: ",1,2147483647,0,false);
    House* h = find_house_by_id(id);
    if(!h || h->landlord_id!=owner->id){
        printf(RED "House not found or not yours.\n" RESET);
        return;
    }
    printf("Status: 0=Available, 1=Rented, 2=Maintenance\n");
    int st = read_int_range("New status: ",0,2,h->status,false);
    h->status = (HouseStatus)st;
    save_houses();
    printf(GREEN "Status updated.\n" RESET);
}

static void landlord_edit_house(User* owner){
    int id = read_int_range("House ID to edit: ",1,2147483647,0,false);
    House* h = find_house_by_id(id);
    if(!h || h->landlord_id!=owner->id){
        printf(RED "House not found or not yours.\n" RESET);
        return;
    }
    char line[600];
    printf(YELLOW "Leave blank to keep current.\n" RESET);

    printf("Title [%s]: ", h->title);
    fflush(stdout);
    if(fgets(line,sizeof(line),stdin)){
        trim_newline(line);
        if(line[0]){
            strncpy(h->title,line,sizeof(h->title)-1);
            h->title[sizeof(h->title)-1] = '\0';
        }
    }

    printf("Address [%s]: ", h->address);
    fflush(stdout);
    if(fgets(line,sizeof(line),stdin)){
        trim_newline(line);
        if(line[0]){
            strncpy(h->address,line,sizeof(h->address)-1);
            h->address[sizeof(h->address)-1] = '\0';
        }
    }

    printf("City [%s]: ", h->city);
    fflush(stdout);
    if(fgets(line,sizeof(line),stdin)){
        trim_newline(line);
        if(line[0]){
            strncpy(h->city,line,sizeof(h->city)-1);
            h->city[sizeof(h->city)-1] = '\0';
        }
    }

    printf("Area [%s]: ", h->area);
    fflush(stdout);
    if(fgets(line,sizeof(line),stdin)){
        trim_newline(line);
        if(line[0]){
            strncpy(h->area,line,sizeof(h->area)-1);
            h->area[sizeof(h->area)-1] = '\0';
        }
    }

    h->bedrooms  = read_int_range("Bedrooms (blank keep): ",0,50,h->bedrooms,true);
    h->bathrooms = read_int_range("Bathrooms (blank keep): ",0,50,h->bathrooms,true);
    h->rent      = read_double_nonneg("Monthly Rent (blank keep): ",h->rent,true);

    printf("Description [current kept if blank]\n> ");
    if(fgets(line,sizeof(line),stdin)){
        trim_newline(line);
        if(line[0]){
            strncpy(h->description,line,sizeof(h->description)-1);
            h->description[sizeof(h->description)-1] = '\0';
        }
    }

    save_houses();
    printf(GREEN "House updated.\n" RESET);
}

static void landlord_delete_house(User* owner){
    int id = read_int_range("House ID to delete: ",1,2147483647,0,false);
    int idx=-1;
    for(int i=0;i<house_count;i++)
        if(houses[i].id==id && houses[i].landlord_id==owner->id){
            idx=i;
            break;
        }
    if(idx<0){
        printf(RED "House not found or not yours.\n" RESET);
        return;
    }

    // Block delete if active rental exists
    for(int r=0;r<rental_count;r++)
        if(rentals[r].house_id==id && rentals[r].is_active){
            printf(RED "Active rental exists; cannot delete.\n" RESET);
            return;
        }

    for(int i=idx;i<house_count-1;i++) houses[i]=houses[i+1];
    house_count--;
    save_houses();
    printf(GREEN "House deleted.\n" RESET);
}

// --------------- Tenant Features ----------
static void tenant_browse_available(void){
    printf(CYAN "\n-- Available Houses --\n" RESET);
    printf("%-4s | %-18s | %-10s | %-10s | %3s | %3s | %-9s\n",
           "ID","Title","City","Area","Bd","Bt","Rent");
    for(int i=0;i<house_count;i++){
        if(houses[i].status==STATUS_AVAILABLE){
            printf("%-4d | %-18s | %-10s | %-10s | %3d | %3d | %9.2f\n",
                   houses[i].id, houses[i].title, houses[i].city, houses[i].area,
                   houses[i].bedrooms, houses[i].bathrooms, houses[i].rent);
        }
    }
}

static void tenant_view_my_rentals(const User* t){
    printf(CYAN "\n-- My Rentals --\n" RESET);
    printf("%-4s | %-18s | %-10s | %-6s | %-9s\n","ID","House","StartDate","Active","Rent");
    for(int i=0;i<rental_count;i++){
        if(rentals[i].tenant_id==t->id){
            printf("%-4d | %-18s | %-10s | %-6s | %9.2f\n",
                   rentals[i].id, rentals[i].house_title, rentals[i].rental_date,
                   rentals[i].is_active?"Yes":"No", rentals[i].monthly_rent);
        }
    }
}

static void tenant_rent_house(User* t){
    int hid = read_int_range("Enter House ID to rent: ",1,2147483647,0,false);
    House* h = find_house_by_id(hid);
    if(!h || h->status!=STATUS_AVAILABLE){
        printf(RED "House not found or not available.\n" RESET);
        return;
    }
    if(rental_count>=MAX_RENTALS){
        printf(RED "Rental capacity reached.\n" RESET);
        return;
    }

    Rental r;
    memset(&r,0,sizeof(r));
    r.id = next_rental_id();
    r.house_id = h->id;
    r.tenant_id= t->id;
    r.landlord_id = h->landlord_id;
    strncpy(r.tenant_name, t->full_name, sizeof(r.tenant_name)-1);
    r.tenant_name[sizeof(r.tenant_name)-1] = '\0';
    strncpy(r.house_title, h->title, sizeof(r.house_title)-1);
    r.house_title[sizeof(r.house_title)-1] = '\0';
    strncpy(r.rental_date, today(), sizeof(r.rental_date)-1);
    r.rental_date[sizeof(r.rental_date)-1] = '\0';
    r.monthly_rent = h->rent;
    r.is_active = true;

    rentals[rental_count++] = r;
    h->status = STATUS_RENTED;
    save_rentals();
    save_houses();
    printf(GREEN "Rental created. Rental ID %d\n" RESET, r.id);
}

static void tenant_end_rental(User* t){
    int rid = read_int_range("Rental ID to end: ",1,2147483647,0,false);
    Rental* r = find_rental_by_id(rid);
    if(!r || r->tenant_id!=t->id){
        printf(RED "Rental not found or not yours.\n" RESET);
        return;
    }
    if(!r->is_active){
        printf(YELLOW "Rental already inactive.\n" RESET);
        return;
    }
    r->is_active=false;
    House* h = find_house_by_id(r->house_id);
    if(h && h->status==STATUS_RENTED) h->status=STATUS_AVAILABLE;
    save_rentals();
    save_houses();
    printf(GREEN "Rental ended.\n" RESET);
}

// --------------- Role Menus ---------------
static void admin_menu(void){
    for(;;){
        clear_screen();
        printf(RED "==================== A D M I N ====================\n" RESET);
        printf("1. List Users\n2. Toggle User Active\n3. Reset User Password\n4. List Houses\n5. List Rentals\n6. Back\n");
        int c = read_int_range("Choice: ",1,6,6,false);
        if(c==1) admin_list_users();
        else if(c==2) admin_toggle_active();
        else if(c==3) admin_reset_password();
        else if(c==4) admin_list_houses();
        else if(c==5) admin_list_rentals();
        else break;
        pause_enter();
    }
}

static void landlord_menu(User* me){
    for(;;){
        clear_screen();
        printf(RED "================== L A N D L O R D =================\n" RESET);
        printf("1. Add House\n2. Edit House\n3. Delete House\n4. Change House Status\n5. My Houses\n6. Back\n");
        int c = read_int_range("Choice: ",1,6,6,false);
        if(c==1) landlord_add_house(me);
        else if(c==2) landlord_edit_house(me);
        else if(c==3) landlord_delete_house(me);
        else if(c==4) landlord_change_status(me);
        else if(c==5) landlord_list_my_houses(me);
        else break;
        pause_enter();
    }
}

static void tenant_menu(User* me){
    for(;;){
        clear_screen();
        printf(RED "=================== T E N A N T ====================\n" RESET);
        printf("1. Browse Available Houses\n2. Rent a House\n3. My Rentals\n4. End Rental\n5. Back\n");
        int c = read_int_range("Choice: ",1,5,5,false);
        if(c==1) tenant_browse_available();
        else if(c==2) tenant_rent_house(me);
        else if(c==3) tenant_view_my_rentals(me);
        else if(c==4) tenant_end_rental(me);
        else break;
        pause_enter();
    }
}

// ---------------- Main Menu ---------------
static int menu_main(void){
    clear_screen();
    printf(RED "======================\n");
    printf("   M A I N  M E N U\n");
    printf(RED "======================\n" RESET);
    printf(GREEN  "1. Login\n" RESET);
    printf(YELLOW "2. Register\n" RESET);
    printf(RED    "3. Exit\n" RESET);
    return read_int_range("Choose an option: ",1,3,3,false);
}

// -------------------- main ----------------
int main(void){
    enable_vt_mode();  // ANSI colors on Windows 10+ terminals
    splash();          // fancy animated welcome

    load_users();
    load_houses();
    load_rentals();

    for(;;){
        int choice = menu_main();
        if(choice==1){
            User* u = authenticate();
            if(u){
                if(u->role==ROLE_ADMIN) admin_menu();
                else if(u->role==ROLE_LANDLORD) landlord_menu(u);
                else tenant_menu(u);
            } else {
                pause_enter();
            }
        } else if(choice==2){
            register_user();
            pause_enter();
        } else {
            printf(GREEN "Goodbye!\n" RESET);
            break;
        }
    }
    return 0;
}
