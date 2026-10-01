#include <stdio.h>

int main() {
    printf("I'll be back");
    printf("\033[2K");
    printf("\033[s");
    printf("\033[10;20H");
    printf("At (10,20)");
    printf("\033[u");
    printf("Back to original");
    return 0;
}
