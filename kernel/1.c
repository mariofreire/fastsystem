int main() {
    __asm__ (
        "movw $0x1234, %ax\n\t"
        "movw %ax, %ds\n\t"
    );

    // Your code here

    return 0;
}