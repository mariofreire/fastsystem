#include <stdio.h>

int main() {
    unsigned short ds_reg;
    asm("mov %%ds, %0" : "=r" (ds_reg));
    printf("Data Segment Register: %04x\n", ds_reg);
    return 0;
}