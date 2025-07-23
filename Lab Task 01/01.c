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
    // Selection Sort for Descending Order
    for (int i = 0; i < n - 1; i++) {
        int max = i;  // Assume current i is the max

        for (int j = i + 1; j < n; j++) {
            if (id[j] > id[max]) {
                max = j;  // Update max if we find a bigger value
            }
        }
        // Swap id[i] and id[max]
        int temp = id[i];
        id[i] = id[max];
        id[max] = temp;
    }
    printf("After Sorting in Descending Order: ");
    printArray(id);
    return 0;
}
