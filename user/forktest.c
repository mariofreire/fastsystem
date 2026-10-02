#include <stdio.h>
#include <unistd.h>

int new_fork(void) 
{
    int pid = fork();
    return pid;
}

int main(void)
{
	int result = new_fork();
	printf("pid: %i\n", result);
	return 0;
}
