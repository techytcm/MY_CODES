#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===== File Definitions =====
#define HOUSE_FILE "houses.txt"
#define LANDLORD_FILE "landlords.txt"
#define TENANT_FILE "tenants.txt"

// ===== Structures =====
typedef struct {
    int id;
    char address[100];
    float rent;
    int is_rented;
} House;

typedef struct {
    char username[30];
    char password[30];
} User;

// ===== Function Declarations =====
int adminLogin();
int landlordLogin();
int tenantLogin();
void tenantRegister();
void addProperty();
void deleteProperty();
void bookRental();
void listProperties();
void landlordDashboard();
void tenantMenu();

// ===== Main Menu =====
int main() {
    int choice;
    while (1) {
        printf("\n==== House Rental System ====\n");
        printf("1. Admin Login\n");
        printf("2. Landlord Login\n");
        printf("3. Tenant Registration\n");
        printf("4. Tenant Login\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if (adminLogin())
                    deleteProperty();
                break;
            case 2:
                if (landlordLogin())
                    landlordDashboard();
                break;
            case 3:
                tenantRegister();
                break;
            case 4:
                if (tenantLogin())
                    tenantMenu();
                break;
            case 5:
                exit(0);
            default:
                printf("Invalid choice.\n");
        }
    }
}

// ===== Admin Login =====
int adminLogin() {
    char username[30], password[30];
    printf("Enter admin username: ");
    scanf("%s", username);
    printf("Enter admin password: ");
    scanf("%s", password);

    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        printf("Admin login successful!\n");
        return 1;
    }
    printf("Invalid admin credentials.\n");
    return 0;
}

// ===== Landlord Login =====
int landlordLogin() {
    char uname[30], pass[30];
    FILE *f = fopen(LANDLORD_FILE, "r");
    User user;
    int found = 0;

    printf("Username: ");
    scanf("%s", uname);
    printf("Password: ");
    scanf("%s", pass);

    while (fscanf(f, "%s %s", user.username, user.password) != EOF) {
        if (strcmp(uname, user.username) == 0 && strcmp(pass, user.password) == 0) {
            found = 1;
            break;
        }
    }

    fclose(f);
    if (found) {
        printf("Landlord login successful!\n");
        return 1;
    }

    printf("Invalid landlord credentials.\n");
    return 0;
}

// ===== Landlord Dashboard =====
void landlordDashboard() {
    int choice;
    printf("1. Add Property\nEnter your choice: ");
    scanf("%d", &choice);
    if (choice == 1)
        addProperty();
}

// ===== Add Property =====
void addProperty() {
    House h;
    FILE *f = fopen(HOUSE_FILE, "a");

    printf("Enter property ID: ");
    scanf("%d", &h.id);
    printf("Enter address: ");
    getchar();
    fgets(h.address, 100, stdin);
    strtok(h.address, "\n");
    printf("Enter rent: ");
    scanf("%f", &h.rent);
    h.is_rented = 0;

    fprintf(f, "%d %s %.2f %d\n", h.id, h.address, h.rent, h.is_rented);
    fclose(f);
    printf("Property added successfully!\n");
}

// ===== Tenant Registration =====
void tenantRegister() {
    User user;
    FILE *f = fopen(TENANT_FILE, "a+");
    int exists = 0;

    printf("Choose a username: ");
    scanf("%s", user.username);
    printf("Choose a password: ");
    scanf("%s", user.password);

    // Check if username already exists
    User temp;
    while (fscanf(f, "%s %s", temp.username, temp.password) != EOF) {
        if (strcmp(temp.username, user.username) == 0) {
            exists = 1;
            break;
        }
    }

    if (exists) {
        printf("Username already taken.\n");
    } else {
        fprintf(f, "%s %s\n", user.username, user.password);
        printf("Registration successful!\n");
    }
    fclose(f);
}

// ===== Tenant Login =====
int tenantLogin() {
    char uname[30], pass[30];
    FILE *f = fopen(TENANT_FILE, "r");
    User user;
    int found = 0;

    printf("Username: ");
    scanf("%s", uname);
    printf("Password: ");
    scanf("%s", pass);

    while (fscanf(f, "%s %s", user.username, user.password) != EOF) {
        if (strcmp(uname, user.username) == 0 && strcmp(pass, user.password) == 0) {
            found = 1;
            break;
        }
    }

    fclose(f);
    if (found) {
        printf("Tenant login successful!\n");
        return 1;
    }

    printf("Invalid tenant credentials.\n");
    return 0;
}

// ===== Tenant Menu =====
void tenantMenu() {
    int choice;
    printf("1. Browse and Book Property\nEnter your choice: ");
    scanf("%d", &choice);
    if (choice == 1)
        bookRental();
}

// ===== List Properties =====
void listProperties() {
    FILE *f = fopen(HOUSE_FILE, "r");
    House h;

    printf("\nAvailable Properties:\n");
    while (fscanf(f, "%d %[^\n] %f %d", &h.id, h.address, &h.rent, &h.is_rented) != EOF) {
        if (!h.is_rented)
            printf("ID: %d | Address: %s | Rent: %.2f\n", h.id, h.address, h.rent);
    }
    fclose(f);
}

// ===== Book a Rental =====
void bookRental() {
    int id;
    listProperties();
    printf("Enter ID of property to rent: ");
    scanf("%d", &id);

    FILE *f = fopen(HOUSE_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    House h;
    int found = 0;

    while (fscanf(f, "%d %[^\n] %f %d", &h.id, h.address, &h.rent, &h.is_rented) != EOF) {
        if (h.id == id && h.is_rented == 0) {
            h.is_rented = 1;
            found = 1;
        }
        fprintf(temp, "%d %s %.2f %d\n", h.id, h.address, h.rent, h.is_rented);
    }

    fclose(f);
    fclose(temp);
    remove(HOUSE_FILE);
    rename("temp.txt", HOUSE_FILE);

    if (found)
        printf("Property booked successfully!\n");
    else
        printf("Invalid or already rented property.\n");
}

// ===== Admin Deletes Property =====
void deleteProperty() {
    int id;
    printf("Enter Property ID to delete: ");
    scanf("%d", &id);

    FILE *f = fopen(HOUSE_FILE, "r");
    FILE *temp = fopen("temp.txt", "w");
    House h;
    int found = 0;

    while (fscanf(f, "%d %[^\n] %f %d", &h.id, h.address, &h.rent, &h.is_rented) != EOF) {
        if (h.id == id) {
            found = 1;
            continue;
        }
        fprintf(temp, "%d %s %.2f %d\n", h.id, h.address, h.rent, h.is_rented);
    }

    fclose(f);
    fclose(temp);
    remove(HOUSE_FILE);
    rename("temp.txt", HOUSE_FILE);

    if (found)
        printf("Property deleted.\n");
    else
        printf("Property not found.\n");
}
