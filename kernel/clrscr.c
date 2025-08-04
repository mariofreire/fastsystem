#include <stdio.h>

int main() {
    // Clear the screen using VT100 escape sequence
    printf("\033[2J");

    // Optionally move the cursor to the home position (top-left corner)
    printf("\033[H");

    return 0;
}