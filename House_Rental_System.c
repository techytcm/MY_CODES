#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_NAME "houses.txt"

typedef struct {
    int id;
    char owner[50];
    char address[100];
    float rent;
    int is_rented;
} House;

// Function declarations
void adminMenu();
void userMenu();
int adminLogin();

void addHouse();
void viewHouses();
void searchHouse();
void rentHouse();
void deleteHouse();
void modifyHouse();

int main() {
    int choice;

    do {
        printf("\n===== Welcome to House Rental System =====\n");
        printf("1. Admin Login\n");
        printf("2. User Access\n");
        printf("3. Exit\n");
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
                userMenu();
                break;
            case 3:
                printf("Exiting system. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 3);

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

// === Add House ===
void addHouse() {
    House h;
    FILE *fp = fopen(FILE_NAME, "ab");

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
    FILE *fp = fopen(FILE_NAME, "rb");
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
    FILE *fp = fopen(FILE_NAME, "rb");
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
    FILE *fp = fopen(FILE_NAME, "rb+");

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
    FILE *fp = fopen(FILE_NAME, "rb");
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
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found)
        printf("House deleted.\n");
    else
        printf("House ID not found.\n");
}

// === Modify House ===
void modifyHouse() {
    int id, found = 0;
    House h;
    FILE *fp = fopen(FILE_NAME, "rb+");

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
}
// End of file
// This code implements a simple house rental system with admin and user functionalities.