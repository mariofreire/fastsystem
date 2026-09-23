#include <stdio.h>

int main() {
    printf("\033[s");           // Save current position
    printf("\033[10;20H");      // Move cursor to (10,20)
    printf("At (10,20)");
    printf("\033[u");           // Restore original position
    printf("Back to original");

    return 0;
}