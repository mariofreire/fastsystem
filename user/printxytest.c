#include <stdio.h>

int main() {
    // Move cursor to row 5, column 10
    printf("\033[5;10H");

    // Print something at that position
    printf("Hello at (5,10)");

    return 0;
}