#include <stdio.h>

#define n 8  // Total number of elements

// Function to print array
void printArray(int arr[]) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Initialize array with your student ID values
    int id[n] = {2, 4, 2, 3, 5, 5, 0, 7};
    printf("My Student ID: ");
    printArray(id);
    // Bubble Sort for Ascending Order
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (id[j] > id[j + 1]) {
                // Swap if current is greater than next
                int temp = id[j];
                id[j] = id[j + 1];
                id[j + 1] = temp;
            }
        }
    }
    printf("After Sorting in Ascending Order: ");
    printArray(id);

    return 0;
}
