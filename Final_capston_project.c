

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#define MAX_USERS    100
#define MAX_HOUSES   200
#define MAX_RENTALS  400


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
    char date_added[20];
} House;

typedef struct {
    int id;
    int house_id;
    int tenant_id;
    int landlord_id;
    char tenant_name[100];
    char house_title[100];
    char rental_date[20];
    double monthly_rent;
    bool is_active;
} Rental;

User users[MAX_USERS];
House houses[MAX_HOUSES];
Rental rentals[MAX_RENTALS];
int user_count = 0;
int house_count = 0;
int rental_count = 0;

#ifndef _WIN32
int getch() {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}
#endif

void trim_newline(char* s) {
    if (s) {
        int len = strlen(s);
        while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
            s[len-1] = '\0';
            len--;
        }
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause_enter() {
    printf("\nPress Enter to continue...");
    clear_input_buffer();
}

void safe_input_line(const char* prompt, char* buf, int n) {
    printf("%s", prompt);
    if (fgets(buf, n, stdin)) {
        trim_newline(buf);
    } else {
        buf[0] = '\0';
    }
}

void get_password(const char* prompt, char* buf, int n) {
    printf("%s", prompt);
    fflush(stdout);

    int i = 0;
    char ch;

    while (1) {
        ch = getch();

        if (ch == '\r' || ch == '\n') { // Enter key
            buf[i] = '\0';
            printf("\n");
            break;
        } else if (ch == 8 || ch == 127) { // Backspace (Windows: 8, Unix: 127)
            if (i > 0) {
                i--;
                printf("\b \b"); // Move back, print space, move back
                fflush(stdout);
            }
        } else if (ch >= 32 && ch <= 126 && i < n - 1) { // Printable characters
            buf[i++] = ch;
            printf("*");
            fflush(stdout);
        }
    }
}

bool is_valid_email(const char* email) {
    if (!email || strlen(email) < 5) return false;

    int at_count = 0;
    int at_pos = -1;
    int len = strlen(email);
    bool dot_after_at = false;

    if (email[0] == '@' || email[0] == '.' ||
        email[len-1] == '@' || email[len-1] == '.') {
        return false;
    }

    for (int i = 0; i < len; i++) {
        if (email[i] == '@') {
            at_count++;
            at_pos = i;
        }
    }


    if (at_count != 1 || at_pos <= 0 || at_pos >= len - 2) {
        return false;
    }

    for (int i = at_pos + 1; i < len; i++) {
        if (email[i] == '.') {
            if (i == at_pos + 1 || i == len - 1) return false; // dot right after @ or at end
            dot_after_at = true;
            break;
        }
    }

    return dot_after_at;
}


bool is_valid_phone(const char* phone) {
    if (!phone) return false;

    int len = strlen(phone);
    if (len < 7 || len > 15) return false;

    int digit_count = 0;
    for (int i = 0; i < len; i++) {
        if (isdigit(phone[i])) {
            digit_count++;
        } else if (phone[i] != '+' && phone[i] != '-' && phone[i] != ' ' &&
                   phone[i] != '(' && phone[i] != ')') {
            return false;
        }
    }

    return digit_count >= 7;
}

int safe_read_int(const char* prompt) {
    char line[64];
    int value;

    while (1) {
        safe_input_line(prompt, line, sizeof(line));
        if (strlen(line) == 0) {
            printf("Please enter a value.\n");
            continue;
        }
        if (sscanf(line, "%d", &value) == 1) {
            return value;
        }
        printf("Invalid integer. Please try again.\n");
    }
}

int safe_read_int_range(const char* prompt, int min, int max) {
    int value;

    while (1) {
        value = safe_read_int(prompt);
        if (value >= min && value <= max) {
            return value;
        }
        printf("Value must be between %d and %d. Please try again.\n", min, max);
    }
}

double safe_read_double(const char* prompt) {
    char line[64];
    double value;

    while (1) {
        safe_input_line(prompt, line, sizeof(line));
        if (strlen(line) == 0) {
            printf("Please enter a value.\n");
            continue;
        }
        if (sscanf(line, "%lf", &value) == 1 && value >= 0) {
            return value;
        }
        printf("Invalid number (must be positive). Please try again.\n");
    }
}

const char* today() {
    static char date[20];
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    strftime(date, sizeof(date), "%Y-%m-%d", tm_info);
    return date;
}

const char* role_str(UserRole role) {
    switch (role) {
        case ROLE_ADMIN: return "Admin";
        case ROLE_LANDLORD: return "Landlord";
        case ROLE_TENANT: return "Tenant";
        default: return "Unknown";
    }
}

const char* status_str(HouseStatus status) {
    switch (status) {
        case STATUS_AVAILABLE: return "Available";
        case STATUS_RENTED: return "Rented";
        case STATUS_MAINTENANCE: return "Maintenance";
        default: return "Unknown";
    }
}

int next_user_id() {
    int max_id = 0;
    for (int i = 0; i < user_count; i++) {
        if (users[i].id > max_id) {
            max_id = users[i].id;
        }
    }
    return max_id + 1;
}

int next_house_id() {
    int max_id = 0;
    for (int i = 0; i < house_count; i++) {
        if (houses[i].id > max_id) {
            max_id = houses[i].id;
        }
    }
    return max_id + 1;
}

int next_rental_id() {
    int max_id = 0;
    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].id > max_id) {
            max_id = rentals[i].id;
        }
    }
    return max_id + 1;
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}


void load_users() {
    FILE* file = fopen("users.txt", "r");
    if (!file) {
        printf("Note: users.txt not found. Starting with empty user database.\n");
        return;
    }

    char line[1024];
    user_count = 0;

    while (fgets(line, sizeof(line), file) && user_count < MAX_USERS) {
        User user;
        int role, active;

        if (sscanf(line, "%d|%49[^|]|%49[^|]|%99[^|]|%99[^|]|%19[^|]|%d|%d",
                  &user.id, user.username, user.password, user.full_name,
                  user.email, user.phone, &role, &active) == 8) {
            user.role = (UserRole)role;
            user.is_active = (bool)active;
            users[user_count++] = user;
        }
    }
    fclose(file);
    printf("Loaded %d users from file.\n", user_count);
}

void save_users() {
    FILE* file = fopen("users.txt", "w");
    if (!file) {
        printf("Error: Cannot save users to file.\n");
        return;
    }

    for (int i = 0; i < user_count; i++) {
        fprintf(file, "%d|%s|%s|%s|%s|%s|%d|%d\n",
                users[i].id, users[i].username, users[i].password,
                users[i].full_name, users[i].email, users[i].phone,
                users[i].role, users[i].is_active);
    }
    fclose(file);
}

void load_houses() {
    FILE* file = fopen("houses.txt", "r");
    if (!file) {
        printf("Note: houses.txt not found. Starting with empty house database.\n");
        return;
    }

    char line[2048];
    house_count = 0;

    while (fgets(line, sizeof(line), file) && house_count < MAX_HOUSES) {
        House house;
        int status;

        if (sscanf(line, "%d|%99[^|]|%199[^|]|%49[^|]|%49[^|]|%d|%d|%lf|%499[^|]|%d|%99[^|]|%d|%19[^\n]",
                  &house.id, house.title, house.address, house.city, house.area,
                  &house.bedrooms, &house.bathrooms, &house.rent, house.description,
                  &house.landlord_id, house.landlord_name, &status, house.date_added) == 13) {
            house.status = (HouseStatus)status;
            houses[house_count++] = house;
        }
    }
    fclose(file);
    printf("Loaded %d houses from file.\n", house_count);
}

void save_houses() {
    FILE* file = fopen("houses.txt", "w");
    if (!file) {
        printf("Error: Cannot save houses to file.\n");
        return;
    }

    for (int i = 0; i < house_count; i++) {
        fprintf(file, "%d|%s|%s|%s|%s|%d|%d|%.2f|%s|%d|%s|%d|%s\n",
                houses[i].id, houses[i].title, houses[i].address, houses[i].city,
                houses[i].area, houses[i].bedrooms, houses[i].bathrooms,
                houses[i].rent, houses[i].description, houses[i].landlord_id,
                houses[i].landlord_name, houses[i].status, houses[i].date_added);
    }
    fclose(file);
}

void load_rentals() {
    FILE* file = fopen("rentals.txt", "r");
    if (!file) {
        printf("Note: rentals.txt not found. Starting with empty rental database.\n");
        return;
    }

    char line[1024];
    rental_count = 0;

    while (fgets(line, sizeof(line), file) && rental_count < MAX_RENTALS) {
        Rental rental;
        int active;

        if (sscanf(line, "%d|%d|%d|%d|%99[^|]|%99[^|]|%19[^|]|%lf|%d",
                  &rental.id, &rental.house_id, &rental.tenant_id, &rental.landlord_id,
                  rental.tenant_name, rental.house_title, rental.rental_date,
                  &rental.monthly_rent, &active) == 9) {
            rental.is_active = (bool)active;
            rentals[rental_count++] = rental;
        }
    }
    fclose(file);
    printf("Loaded %d rentals from file.\n", rental_count);
}

void save_rentals() {
    FILE* file = fopen("rentals.txt", "w");
    if (!file) {
        printf("Error: Cannot save rentals to file.\n");
        return;
    }

    for (int i = 0; i < rental_count; i++) {
        fprintf(file, "%d|%d|%d|%d|%s|%s|%s|%.2f|%d\n",
                rentals[i].id, rentals[i].house_id, rentals[i].tenant_id,
                rentals[i].landlord_id, rentals[i].tenant_name, rentals[i].house_title,
                rentals[i].rental_date, rentals[i].monthly_rent, rentals[i].is_active);
    }
    fclose(file);
}


User* find_user_by_id(int id) {
    for (int i = 0; i < user_count; i++) {
        if (users[i].id == id) {
            return &users[i];
        }
    }
    return NULL;
}

User* find_user_by_username(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

House* find_house_by_id(int id) {
    for (int i = 0; i < house_count; i++) {
        if (houses[i].id == id) {
            return &houses[i];
        }
    }
    return NULL;
}

Rental* find_rental_by_id(int id) {
    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].id == id) {
            return &rentals[i];
        }
    }
    return NULL;
}


User* authenticate() {
    char username[64], password[64];

    safe_input_line("Username: ", username, sizeof(username));
    get_password("Password: ", password, sizeof(password));

    User* user = find_user_by_username(username);
    if (user && strcmp(user->password, password) == 0) {
        if (!user->is_active) {
            printf("Account is inactive. Please contact administrator.\n");
            return NULL;
        }
        printf("Welcome, %s!\n", user->full_name);
        return user;
    }

    printf("Invalid username or password.\n");
    return NULL;
}

void register_user() {
    if (user_count >= MAX_USERS) {
        printf("Cannot register more users. System limit reached.\n");
        return;
    }

    User new_user;
    new_user.id = next_user_id();


    do {
        safe_input_line("Username (3-49 chars): ", new_user.username, sizeof(new_user.username));
        if (strlen(new_user.username) < 3) {
            printf("Username must be at least 3 characters long.\n");
            continue;
        }
        if (find_user_by_username(new_user.username)) {
            printf("Username already exists. Please choose another.\n");
            continue;
        }
        break;
    } while (1);

    do {
        get_password("Password (4-49 chars): ", new_user.password, sizeof(new_user.password));
        if (strlen(new_user.password) < 4) {
            printf("Password must be at least 4 characters long.\n");
            continue;
        }
        break;
    } while (1);

    safe_input_line("Full name: ", new_user.full_name, sizeof(new_user.full_name));


    do {
        safe_input_line("Email: ", new_user.email, sizeof(new_user.email));
        if (!is_valid_email(new_user.email)) {
            printf("Invalid email format. Please try again.\n");
        } else {
            break;
        }
    } while (1);

    do {
        safe_input_line("Phone: ", new_user.phone, sizeof(new_user.phone));
        if (!is_valid_phone(new_user.phone)) {
            printf("Invalid phone number. Please try again.\n");
        } else {
            break;
        }
    } while (1);

    printf("\nSelect role:\n");
    printf("0 - Admin\n");
    printf("1 - Landlord\n");
    printf("2 - Tenant\n");
    new_user.role = (UserRole)safe_read_int_range("Role: ", 0, 2);

    new_user.is_active = true;

    users[user_count++] = new_user;
    save_users();

    printf("\nUser registered successfully!\n");
    printf("User ID: %d\n", new_user.id);
    printf("Username: %s\n", new_user.username);
    printf("Role: %s\n", role_str(new_user.role));
}

void admin_list_users() {
    printf("\n=== USERS LIST ===\n");
    if (user_count == 0) {
        printf("No users found.\n");
        return;
    }

    printf("%-4s | %-15s | %-25s | %-10s | %-6s\n", "ID", "Username", "Full Name", "Role", "Active");
    printf("-----|-----------------|---------------------------|------------|--------\n");

    for (int i = 0; i < user_count; i++) {
        printf("%-4d | %-15s | %-25s | %-10s | %-6s\n",
               users[i].id, users[i].username, users[i].full_name,
               role_str(users[i].role), users[i].is_active ? "Yes" : "No");
    }
}

void admin_toggle_active() {
    if (user_count == 0) {
        printf("No users available.\n");
        return;
    }

    admin_list_users();
    int id = safe_read_int("\nEnter user ID to toggle active status: ");
    User* user = find_user_by_id(id);

    if (!user) {
        printf("User not found.\n");
        return;
    }

    user->is_active = !user->is_active;
    save_users();

    printf("User '%s' active status changed to: %s\n",
           user->username, user->is_active ? "Active" : "Inactive");
}

void admin_reset_password() {
    if (user_count == 0) {
        printf("No users available.\n");
        return;
    }

    admin_list_users();
    int id = safe_read_int("\nEnter user ID to reset password: ");
    User* user = find_user_by_id(id);

    if (!user) {
        printf("User not found.\n");
        return;
    }

    strcpy(user->password, "1234");
    save_users();

    printf("Password for user '%s' has been reset to '1234'\n", user->username);
}

void admin_list_houses() {
    printf("\n=== HOUSES LIST ===\n");
    if (house_count == 0) {
        printf("No houses found.\n");
        return;
    }

    printf("%-4s | %-20s | %-12s | %-12s | %-2s | %-2s | %-12s | %8s\n",
           "ID", "Title", "City", "Area", "Bd", "Bt", "Status", "Rent");
    printf("-----|----------------------|--------------|--------------|----|----|--------------|----------\n");

    for (int i = 0; i < house_count; i++) {
        printf("%-4d | %-20s | %-12s | %-12s | %2d | %2d | %-12s | %8.2f\n",
               houses[i].id, houses[i].title, houses[i].city, houses[i].area,
               houses[i].bedrooms, houses[i].bathrooms, status_str(houses[i].status),
               houses[i].rent);
    }
}

void admin_list_rentals() {
    printf("\n=== RENTALS LIST ===\n");
    if (rental_count == 0) {
        printf("No rentals found.\n");
        return;
    }

    printf("%-4s | %-20s | %-20s | %-12s | %-6s | %8s\n",
           "ID", "Tenant", "House", "Start Date", "Active", "Rent");
    printf("-----|----------------------|----------------------|--------------|--------|----------\n");

    for (int i = 0; i < rental_count; i++) {
        printf("%-4d | %-20s | %-20s | %-12s | %-6s | %8.2f\n",
               rentals[i].id, rentals[i].tenant_name, rentals[i].house_title,
               rentals[i].rental_date, rentals[i].is_active ? "Yes" : "No",
               rentals[i].monthly_rent);
    }
}

void landlord_list_my_houses(User* owner) {
    printf("\n=== MY HOUSES (%s) ===\n", owner->full_name);

    int count = 0;
    printf("%-4s | %-20s | %-12s | %-12s | %-2s | %-2s | %-12s | %8s\n",
           "ID", "Title", "City", "Area", "Bd", "Bt", "Status", "Rent");
    printf("-----|----------------------|--------------|--------------|----|----|--------------|----------\n");

    for (int i = 0; i < house_count; i++) {
        if (houses[i].landlord_id == owner->id) {
            printf("%-4d | %-20s | %-12s | %-12s | %2d | %2d | %-12s | %8.2f\n",
                   houses[i].id, houses[i].title, houses[i].city, houses[i].area,
                   houses[i].bedrooms, houses[i].bathrooms, status_str(houses[i].status),
                   houses[i].rent);
            count++;
        }
    }

    if (count == 0) {
        printf("No houses found.\n");
    }
}

void landlord_add_house(User* owner) {
    if (house_count >= MAX_HOUSES) {
        printf("Cannot add more houses. System limit reached.\n");
        return;
    }

    House new_house;
    new_house.id = next_house_id();

    safe_input_line("House Title: ", new_house.title, sizeof(new_house.title));
    safe_input_line("Address: ", new_house.address, sizeof(new_house.address));
    safe_input_line("City: ", new_house.city, sizeof(new_house.city));
    safe_input_line("Area/Neighborhood: ", new_house.area, sizeof(new_house.area));

    new_house.bedrooms = safe_read_int_range("Number of Bedrooms (0-50): ", 0, 50);
    new_house.bathrooms = safe_read_int_range("Number of Bathrooms (0-50): ", 0, 50);
    new_house.rent = safe_read_double("Monthly Rent: ");

    safe_input_line("Description: ", new_house.description, sizeof(new_house.description));

    new_house.landlord_id = owner->id;
    strcpy(new_house.landlord_name, owner->full_name);
    strcpy(new_house.date_added, today());
    new_house.status = STATUS_AVAILABLE;

    houses[house_count++] = new_house;
    save_houses();

    printf("\nHouse added successfully!\n");
    printf("House ID: %d\n", new_house.id);
    printf("Title: %s\n", new_house.title);
    printf("Rent: %.2f\n", new_house.rent);
}

void landlord_change_status(User* owner) {
    landlord_list_my_houses(owner);

    if (house_count == 0) return;

    int id = safe_read_int("\nEnter house ID to change status: ");
    House* house = find_house_by_id(id);

    if (!house || house->landlord_id != owner->id) {
        printf("House not found or you don't own this house.\n");
        return;
    }

    printf("\nCurrent status: %s\n", status_str(house->status));
    printf("Select new status:\n");
    printf("0 - Available\n");
    printf("1 - Rented\n");
    printf("2 - Maintenance\n");

    int new_status = safe_read_int_range("New status: ", 0, 2);
    house->status = (HouseStatus)new_status;

    save_houses();
    printf("House status updated to: %s\n", status_str(house->status));
}

void landlord_edit_house(User* owner) {
    landlord_list_my_houses(owner);

    if (house_count == 0) return;

    int id = safe_read_int("\nEnter house ID to edit: ");
    House* house = find_house_by_id(id);

    if (!house || house->landlord_id != owner->id) {
        printf("House not found or you don't own this house.\n");
        return;
    }

    char temp[500];

    printf("\nCurrent house details:\n");
    printf("Title: %s\n", house->title);
    printf("Address: %s\n", house->address);
    printf("City: %s\n", house->city);
    printf("Area: %s\n", house->area);
    printf("Bedrooms: %d\n", house->bedrooms);
    printf("Bathrooms: %d\n", house->bathrooms);
    printf("Rent: %.2f\n", house->rent);
    printf("Description: %s\n", house->description);

    printf("\n--- Edit House (Leave blank to keep current value) ---\n");

    printf("Title [%s]: ", house->title);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            strcpy(house->title, temp);
        }
    }

    printf("Address [%s]: ", house->address);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            strcpy(house->address, temp);
        }
    }

    printf("City [%s]: ", house->city);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            strcpy(house->city, temp);
        }
    }

    printf("Area [%s]: ", house->area);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            strcpy(house->area, temp);
        }
    }

    printf("Bedrooms [%d]: ", house->bedrooms);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            int bedrooms = atoi(temp);
            if (bedrooms >= 0 && bedrooms <= 50) {
                house->bedrooms = bedrooms;
            }
        }
    }

    printf("Bathrooms [%d]: ", house->bathrooms);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            int bathrooms = atoi(temp);
            if (bathrooms >= 0 && bathrooms <= 50) {
                house->bathrooms = bathrooms;
            }
        }
    }

    printf("Rent [%.2f]: ", house->rent);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            double rent = atof(temp);
            if (rent >= 0) {
                house->rent = rent;
            }
        }
    }

    printf("Description [%s]: ", house->description);
    if (fgets(temp, sizeof(temp), stdin)) {
        trim_newline(temp);
        if (strlen(temp) > 0) {
            strcpy(house->description, temp);
        }
    }

    save_houses();
    printf("House updated successfully.\n");
}

void landlord_delete_house(User* owner) {
    landlord_list_my_houses(owner);

    if (house_count == 0) return;

    int id = safe_read_int("\nEnter house ID to delete: ");
    int index = -1;

    for (int i = 0; i < house_count; i++) {
        if (houses[i].id == id && houses[i].landlord_id == owner->id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("House not found or you don't own this house.\n");
        return;
    }


    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].house_id == id && rentals[i].is_active) {
            printf("Cannot delete house with active rentals.\n");
            return;
        }
    }

    printf("Are you sure you want to delete house '%s'? (y/n): ", houses[index].title);
    char confirm[10];
    safe_input_line("", confirm, sizeof(confirm));

    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("Deletion cancelled.\n");
        return;
    }

    for (int i = index; i < house_count - 1; i++) {
        houses[i] = houses[i + 1];
    }
    house_count--;

    save_houses();
    printf("House deleted successfully.\n");
}

void landlord_view_rentals(User* owner) {
    printf("\n=== MY RENTALS (%s) ===\n", owner->full_name);

    int count = 0;
    printf("%-4s | %-20s | %-20s | %-12s | %-6s | %8s\n",
           "ID", "Tenant", "House", "Start Date", "Active", "Rent");
    printf("-----|----------------------|----------------------|--------------|--------|----------\n");

    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].landlord_id == owner->id) {
            printf("%-4d | %-20s | %-20s | %-12s | %-6s | %8.2f\n",
                   rentals[i].id, rentals[i].tenant_name, rentals[i].house_title,
                   rentals[i].rental_date, rentals[i].is_active ? "Yes" : "No",
                   rentals[i].monthly_rent);
            count++;
        }
    }

    if (count == 0) {
        printf("No rentals found.\n");
    }
}

void tenant_browse_available() {
    printf("\n=== AVAILABLE HOUSES ===\n");

    int count = 0;
    printf("%-4s | %-20s | %-12s | %-12s | %-2s | %-2s | %8s | %-15s\n",
           "ID", "Title", "City", "Area", "Bd", "Bt", "Rent", "Landlord");
    printf("-----|----------------------|--------------|--------------|----|----|----------|----------------\n");

    for (int i = 0; i < house_count; i++) {
        if (houses[i].status == STATUS_AVAILABLE) {
            printf("%-4d | %-20s | %-12s | %-12s | %2d | %2d | %8.2f | %-15s\n",
                   houses[i].id, houses[i].title, houses[i].city, houses[i].area,
                   houses[i].bedrooms, houses[i].bathrooms, houses[i].rent,
                   houses[i].landlord_name);
            count++;
        }
    }

    if (count == 0) {
        printf("No available houses found.\n");
    }
}

void tenant_view_house_details() {
    tenant_browse_available();

    if (house_count == 0) return;

    int id = safe_read_int("\nEnter house ID to view details (0 to cancel): ");
    if (id == 0) return;

    House* house = find_house_by_id(id);

    if (!house || house->status != STATUS_AVAILABLE) {
        printf("House not found or not available.\n");
        return;
    }

    printf("\n=== HOUSE DETAILS ===\n");
    printf("ID: %d\n", house->id);
    printf("Title: %s\n", house->title);
    printf("Address: %s\n", house->address);
    printf("City: %s\n", house->city);
    printf("Area: %s\n", house->area);
    printf("Bedrooms: %d\n", house->bedrooms);
    printf("Bathrooms: %d\n", house->bathrooms);
    printf("Monthly Rent: %.2f\n", house->rent);
    printf("Description: %s\n", house->description);
    printf("Landlord: %s\n", house->landlord_name);
    printf("Date Added: %s\n", house->date_added);
    printf("Status: %s\n", status_str(house->status));
}

void tenant_view_my_rentals(User* tenant) {
    printf("\n=== MY RENTALS ===\n");

    int count = 0;
    printf("%-4s | %-20s | %-12s | %-6s | %8s\n",
           "ID", "House", "Start Date", "Active", "Rent");
    printf("-----|----------------------|--------------|--------|----------\n");

    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].tenant_id == tenant->id) {
            printf("%-4d | %-20s | %-12s | %-6s | %8.2f\n",
                   rentals[i].id, rentals[i].house_title, rentals[i].rental_date,
                   rentals[i].is_active ? "Yes" : "No", rentals[i].monthly_rent);
            count++;
        }
    }

    if (count == 0) {
        printf("No rentals found.\n");
    }
}

void tenant_rent_house(User* tenant) {
    tenant_browse_available();

    if (house_count == 0) return;

    int house_id = safe_read_int("\nEnter house ID to rent (0 to cancel): ");
    if (house_id == 0) return;

    House* house = find_house_by_id(house_id);

    if (!house || house->status != STATUS_AVAILABLE) {
        printf("House not found or not available for rent.\n");
        return;
    }

    if (rental_count >= MAX_RENTALS) {
        printf("Cannot create more rentals. System limit reached.\n");
        return;
    }


    for (int i = 0; i < rental_count; i++) {
        if (rentals[i].tenant_id == tenant->id &&
            rentals[i].house_id == house_id &&
            rentals[i].is_active) {
            printf("You already have an active rental for this house.\n");
            return;
        }
    }

    printf("\n=== RENTAL CONFIRMATION ===\n");
    printf("House: %s\n", house->title);
    printf("Address: %s\n", house->address);
    printf("Monthly Rent: %.2f\n", house->rent);
    printf("Landlord: %s\n", house->landlord_name);

    printf("\nConfirm rental? (y/n): ");
    char confirm[10];
    safe_input_line("", confirm, sizeof(confirm));

    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("Rental cancelled.\n");
        return;
    }

    Rental new_rental;
    new_rental.id = next_rental_id();
    new_rental.house_id = house->id;
    new_rental.tenant_id = tenant->id;
    new_rental.landlord_id = house->landlord_id;
    strcpy(new_rental.tenant_name, tenant->full_name);
    strcpy(new_rental.house_title, house->title);
    strcpy(new_rental.rental_date, today());
    new_rental.monthly_rent = house->rent;
    new_rental.is_active = true;

    rentals[rental_count++] = new_rental;
    house->status = STATUS_RENTED;

    save_rentals();
    save_houses();

    printf("\nRental created successfully!\n");
    printf("Rental ID: %d\n", new_rental.id);
    printf("Start Date: %s\n", new_rental.rental_date);
    printf("Monthly Rent: %.2f\n", new_rental.monthly_rent);
}

void tenant_end_rental(User* tenant) {
    tenant_view_my_rentals(tenant);

    if (rental_count == 0) return;

    int rental_id = safe_read_int("\nEnter rental ID to end (0 to cancel): ");
    if (rental_id == 0) return;

    Rental* rental = find_rental_by_id(rental_id);

    if (!rental || rental->tenant_id != tenant->id) {
        printf("Rental not found or you don't own this rental.\n");
        return;
    }

    if (!rental->is_active) {
        printf("Rental is already inactive.\n");
        return;
    }

    printf("Are you sure you want to end rental for '%s'? (y/n): ", rental->house_title);
    char confirm[10];
    safe_input_line("", confirm, sizeof(confirm));

    if (confirm[0] != 'y' && confirm[0] != 'Y') {
        printf("Operation cancelled.\n");
        return;
    }

    rental->is_active = false;

    House* house = find_house_by_id(rental->house_id);
    if (house) {
        house->status = STATUS_AVAILABLE;
    }

    save_rentals();
    save_houses();

    printf("Rental ended successfully.\n");
}


void admin_menu() {
    int choice;

    do {
        clear_screen();
        printf("========== ADMIN MENU ==========\n");
        printf("1. List All Users\n");
        printf("2. Toggle User Active Status\n");
        printf("3. Reset User Password\n");
        printf("4. List All Houses\n");
        printf("5. List All Rentals\n");
        printf("6. Back to Main Menu\n");
        printf("================================\n");

        choice = safe_read_int_range("Select option: ", 1, 6);

        switch (choice) {
            case 1: admin_list_users(); break;
            case 2: admin_toggle_active(); break;
            case 3: admin_reset_password(); break;
            case 4: admin_list_houses(); break;
            case 5: admin_list_rentals(); break;
            case 6: printf("Returning to main menu...\n"); break;
        }

        if (choice != 6) {
            pause_enter();
        }
    } while (choice != 6);
}

void landlord_menu(User* user) {
    int choice;

    do {
        clear_screen();
        printf("======== LANDLORD MENU ========\n");
        printf("Welcome, %s\n", user->full_name);
        printf("1. Add New House\n");
        printf("2. Edit House\n");
        printf("3. Delete House\n");
        printf("4. Change House Status\n");
        printf("5. View My Houses\n");
        printf("6. View My Rentals\n");
        printf("7. Back to Main Menu\n");
        printf("================================\n");

        choice = safe_read_int_range("Select option: ", 1, 7);

        switch (choice) {
            case 1: landlord_add_house(user); break;
            case 2: landlord_edit_house(user); break;
            case 3: landlord_delete_house(user); break;
            case 4: landlord_change_status(user); break;
            case 5: landlord_list_my_houses(user); break;
            case 6: landlord_view_rentals(user); break;
            case 7: printf("Returning to main menu...\n"); break;
        }

        if (choice != 7) {
            pause_enter();
        }
    } while (choice != 7);
}

void tenant_menu(User* user) {
    int choice;

    do {
        clear_screen();
        printf("========= TENANT MENU ==========\n");
        printf("Welcome, %s\n", user->full_name);
        printf("1. Browse Available Houses\n");
        printf("2. View House Details\n");
        printf("3. Rent a House\n");
        printf("4. View My Rentals\n");
        printf("5. End Rental\n");
        printf("6. Back to Main Menu\n");
        printf("================================\n");

        choice = safe_read_int_range("Select option: ", 1, 6);

        switch (choice) {
            case 1: tenant_browse_available(); break;
            case 2: tenant_view_house_details(); break;
            case 3: tenant_rent_house(user); break;
            case 4: tenant_view_my_rentals(user); break;
            case 5: tenant_end_rental(user); break;
            case 6: printf("Returning to main menu...\n"); break;
        }

        if (choice != 6) {
            pause_enter();
        }
    } while (choice != 6);
}

int main_menu() {
    clear_screen();
    printf("====================================\n");
    printf("    HOUSE RENTAL MANAGEMENT SYSTEM  \n");
    printf("====================================\n");
    printf("1. Login\n");
    printf("2. Register New User\n");
    printf("3. Exit\n");
    printf("====================================\n");

    return safe_read_int_range("Select option: ", 1, 3);
}


void create_default_admin() {
    // Check if any admin exists
    for (int i = 0; i < user_count; i++) {
        if (users[i].role == ROLE_ADMIN) {
            return; // Admin exists
        }
    }


    if (user_count < MAX_USERS) {
        User admin;
        admin.id = next_user_id();
        strcpy(admin.username, "admin");
        strcpy(admin.password, "admin123");
        strcpy(admin.full_name, "System Administrator");
        strcpy(admin.email, "admin@system.com");
        strcpy(admin.phone, "1234567890");
        admin.role = ROLE_ADMIN;
        admin.is_active = true;

        users[user_count++] = admin;
        save_users();

        printf("Default admin account created:\n");
        printf("Username: admin\n");
        printf("Password: admin123\n\n");
    }
}

int main() {
    printf("====================================\n");
    printf("  HOUSE RENTAL MANAGEMENT SYSTEM\n");
    printf("====================================\n");
    printf("Loading system data...\n");


    load_users();
    load_houses();
    load_rentals();


    create_default_admin();

    printf("System loaded successfully!\n");
    pause_enter();

    int choice;
    do {
        choice = main_menu();

        switch (choice) {
            case 1: {
                User* user = authenticate();
                if (user) {
                    switch (user->role) {
                        case ROLE_ADMIN:
                            admin_menu();
                            break;
                        case ROLE_LANDLORD:
                            landlord_menu(user);
                            break;
                        case ROLE_TENANT:
                            tenant_menu(user);
                            break;
                    }
                } else {
                    pause_enter();
                }
                break;
            }
            case 2: {
                register_user();
                pause_enter();
                break;
            }
            case 3: {
                printf("Thank you for using the House Rental Management System!\n");
                printf("All data has been saved. Goodbye!\n");
                break;
            }
        }
    } while (choice != 3);

    return 0;
}
