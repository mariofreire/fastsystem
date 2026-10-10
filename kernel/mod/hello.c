// Fast System sample module.
// Build: gcc -m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -c hello.c -o hello.o
// Load:  insmod hello.o

void printk(const char *msg, ...);

const char module_name[] = "hello";

int module_init(void)
{
    printk("hello: module loaded\n");
    return 0;
}

int module_exit(void)
{
    printk("hello: module unloaded\n");
    return 0;
}
