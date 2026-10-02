#include <stdio.h>
#include <string.h>
#include <taskthread.h>

int main() 
{
    struct user_desc u_info;
    char my_thread_data[] = "Data saved in TLS via x86 Assembly!";
    int result;

    memset(&u_info, 0, sizeof(u_info));

    u_info.entry_number = -1;
    u_info.base_addr = (unsigned long)my_thread_data;
    u_info.limit = sizeof(my_thread_data);
    u_info.seg_32bit = 1;
    u_info.contents = 0;
    u_info.read_exec_only = 0;
    u_info.limit_in_pages = 0;
    u_info.seg_not_present = 0;
    u_info.useable = 1;

    printf("[+] Invoking set_thread_area via int 0x80 (Syscall 243)...\n");

    __asm__ __volatile__(
        "int $0x80"
        : "=a" (result)
        : "a" (243),
          "b" (&u_info)
        : "memory"
    );

    if (result < 0 && result > -4096) {
        printf("[-] Syscall failed. Raw error code: %d\n", result);
        return 1;
    }

    printf("[+] Success! Area allocated at GDT entry number: %d\n", u_info.entry_number);

    return 0;
}
