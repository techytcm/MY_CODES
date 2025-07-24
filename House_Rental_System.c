#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// === File Definitions ===
#define HOUSE_FILE "houses.txt"
#define USER_FILE "users.txt"

// === Structures ===
typedef struct {
    int id;
    char owner[50];
    char address[100];
    float rent;
    int is_rented;
} House;

typedef struct {
    char username[30];
    char password[30];
} User;

// === Function Declarations ===
void adminMenu();
void userMenu();
int adminLogin();
void addHouse();
void viewHouses();
void searchHouse();
void rentHouse();
void deleteHouse();
void modifyHouse();

void createAccount();
int userLogin();

// === Main Function ===
int main() {
    int choice;
    do {
        printf("\n===== Welcome to House Rental System =====\n");
        printf("1. Admin Login\n");
        printf("2. User Login\n");
        printf("3. Create User Account\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();

        switch(choice) {
            case 1:
                if (adminLogin()) {
                    adminMenu();
                } else {
                    printf("Login failed. Access denied.\n");
                }
                break;
            case 2:
                if (userLogin()) {
                    userMenu();
                }
                break;
            case 3:
                createAccount();
                break;
            case 4:
                printf("Exiting system. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 4);

    return 0;
}

// === Admin Login ===
int adminLogin() {
    char username[20], password[20];
    printf("\n--- Admin Login ---\n");
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        return 1;
    }
    return 0;
}

// === Admin Menu ===
void adminMenu() {
    int choice;
    do {
        printf("\n--- Admin Menu ---\n");
        printf("1. Add House\n");
        printf("2. Delete House\n");
        printf("3. Modify House\n");
        printf("4. View All Houses\n");
        printf("5. Logout\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
        switch(choice) {
            case 1: addHouse(); break;
            case 2: deleteHouse(); break;
            case 3: modifyHouse(); break;
            case 4: viewHouses(); break;
            case 5: printf("Logging out...\n"); break;
            default: printf("Invalid option.\n");
        }
    } while(choice != 5);
}

// === User Menu ===
void userMenu() {
    int choice;
    do {
        printf("\n--- User Menu ---\n");
        printf("1. View All Houses\n");
        printf("2. Search House by ID\n");
        printf("3. Rent a House\n");
        printf("4. Back to Main Menu\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1: viewHouses(); break;
            case 2: searchHouse(); break;
            case 3: rentHouse(); break;
            case 4: printf("Returning...\n"); break;
            default: printf("Invalid option.\n");
        }
    } while(choice != 4);
}

// === Create User Account ===
void createAccount() {
    FILE *fp = fopen(USER_FILE, "ab");
    if (!fp) {
        printf("Error opening user file.\n");
        return;
    }

    User newUser;
    printf("\n--- Create New Account ---\n");
    getchar();
    printf("Enter username: ");
    fgets(newUser.username, sizeof(newUser.username), stdin);
    newUser.username[strcspn(newUser.username, "\n")] = 0;

    printf("Enter password: ");
    fgets(newUser.password, sizeof(newUser.password), stdin);
    newUser.password[strcspn(newUser.password, "\n")] = 0;

    fwrite(&newUser, sizeof(User), 1, fp);
    fclose(fp);
    printf("Account created successfully!\n");
}

// === User Login ===
int userLogin() {
    char uname[30], pword[30];
    User u;
    int found = 0;

    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) {
        printf("No user data found. Please create an account first.\n");
        return 0;
    }

    getchar(); // clear buffer
    printf("\n--- User Login ---\n");
    printf("Username: ");
    fgets(uname, sizeof(uname), stdin);
    uname[strcspn(uname, "\n")] = 0;

    printf("Password: ");
    fgets(pword, sizeof(pword), stdin);
    pword[strcspn(pword, "\n")] = 0;

    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.username, uname) == 0 && strcmp(u.password, pword) == 0) {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (found) {
        printf("Login successful!\n");
        return 1;
    } else {
        printf("Invalid username or password.\n");
        return 0;
    }
}

// === Add House ===
void addHouse() {
    House h;
    FILE *fp = fopen(HOUSE_FILE, "ab");

    if (!fp) {
        printf("Error opening file.\n");
        return;
    }
    printf("Enter House ID: ");
    scanf("%d", &h.id); getchar();
    printf("Enter Owner Name: ");
    fgets(h.owner, sizeof(h.owner), stdin);
    h.owner[strcspn(h.owner, "\n")] = 0;
    printf("Enter Address: ");
    fgets(h.address, sizeof(h.address), stdin);
    h.address[strcspn(h.address, "\n")] = 0;
    printf("Enter Monthly Rent: ");
    scanf("%f", &h.rent);
    h.is_rented = 0;
    fwrite(&h, sizeof(House), 1, fp);
    fclose(fp);
    printf("House added.\n");
}

// === View Houses ===
void viewHouses() {
    House h;
    FILE *fp = fopen(HOUSE_FILE, "rb");
    if (!fp) {
        printf("No data found.\n");
        return;
    }
    printf("\n--- All Houses ---\n");
    while (fread(&h, sizeof(House), 1, fp)) {
        printf("ID: %d | Owner: %s | Address: %s | Rent: %.2f | Status: %s\n",
               h.id, h.owner, h.address, h.rent,
               h.is_rented ? "Rented" : "Available");
    }
    fclose(fp);
}

// === Search House by ID ===
void searchHouse() {
    int id, found = 0;
    House h;
    FILE *fp = fopen(HOUSE_FILE, "rb");
    if (!fp) {
        printf("File error.\n");
        return;
    }
    printf("Enter House ID: ");
    scanf("%d", &id);

    while (fread(&h, sizeof(House), 1, fp)) {
        if (h.id == id) {
            printf("House Found:\n");
            printf("Owner: %s\nAddress: %s\nRent: %.2f\nStatus: %s\n",
                   h.owner, h.address, h.rent, h.is_rented ? "Rented" : "Available");
            found = 1;
            break;
        }
    }
    if (!found) printf("House not found.\n");

    fclose(fp);
}

// === Rent House ===
void rentHouse() {
    int id, found = 0;
    House h;
    FILE *fp = fopen(HOUSE_FILE, "rb+");

    if (!fp) {
        printf("Error opening file.\n");
        return;
    }

    printf("Enter ID of house to rent: ");
    scanf("%d", &id);

    while (fread(&h, sizeof(House), 1, fp)) {
        if (h.id == id) {
            found = 1;
            if (h.is_rented) {
                printf("House is already rented.\n");
            } else {
                h.is_rented = 1;
                fseek(fp, -sizeof(House), SEEK_CUR);
                fwrite(&h, sizeof(House), 1, fp);
                printf("House rented successfully!\n");
            }
            break;
        }
    }

    if (!found) printf("House ID not found.\n");

    fclose(fp);
}

// === Delete House ===
void deleteHouse() {
    int id, found = 0;
    House h;
    FILE *fp = fopen(HOUSE_FILE, "rb");
    FILE *temp = fopen("temp.txt", "wb");

    if (!fp || !temp) {
        printf("File error.\n");
        return;
    }

    printf("Enter House ID to delete: ");
    scanf("%d", &id);

    while (fread(&h, sizeof(House), 1, fp)) {
        if (h.id == id) {
            found = 1;
            continue;
        }
        fwrite(&h, sizeof(House), 1, temp);
    }
    fclose(fp);
    fclose(temp);
    remove(HOUSE_FILE);
    rename("temp.txt", HOUSE_FILE);

    if (found)
        printf("House deleted.\n");
    else
        printf("House ID not found.\n");
}

// === Modify House ===
void modifyHouse() {
    int id, found = 0;
    House h;
    FILE *fp = fopen(HOUSE_FILE, "rb+");

    if (!fp) {
        printf("File error.\n");
        return;
    }
    printf("Enter House ID to modify: ");
    scanf("%d", &id);
    getchar();
    while (fread(&h, sizeof(House), 1, fp)) {
        if (h.id == id) {
            found = 1;
            printf("Enter new Owner Name: ");
            fgets(h.owner, sizeof(h.owner), stdin);
            h.owner[strcspn(h.owner, "\n")] = 0;

            printf("Enter new Address: ");
            fgets(h.address, sizeof(h.address), stdin);
            h.address[strcspn(h.address, "\n")] = 0;

            printf("Enter new Rent: ");
            scanf("%f", &h.rent);

            fseek(fp, -sizeof(House), SEEK_CUR);
            fwrite(&h, sizeof(House), 1, fp);

            printf("House details updated.\n");
            break;
        }
    }
    if (!found)
        printf("House ID not found.\n");
    fclose(fp);