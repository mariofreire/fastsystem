// Fast System Task Thread
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include "taskthread.h"
#include "syscall.h"

int pthread_join(unsigned long thread, void **result)
{
	syscall_regs_t r;
	r = syscall2(390, thread, (unsigned long)result);
	return (int)r.eax;
}

void task_terminate(unsigned long task,	unsigned long signal)
{
	syscall2(238, task,	signal);
}

int get_task_priority(int type, unsigned long task)
{
	syscall_regs_t r;
	r = syscall2(96, (unsigned long)type, task);
	return (int)r.eax;
}

int set_task_priority(int type, unsigned long task,	unsigned long priority)
{
	syscall_regs_t r;
	r = syscall3(97, (unsigned long)type, task, priority);
	return (int)r.eax;
}

int createthread(task_t *handle_instance,
                 const char *name,
                 int attributes,
                 int stack_size,
                 unsigned long *start_address,
                 void *param,
                 int flags,
                 int *new_thread_id)
{
	syscall_regs_t r;
    createthread_args_t args = {
        .handle_instance = handle_instance,
        .name = name,
        .attributes = attributes,
        .stack_size = stack_size,
        .start_address = start_address,
        .param = param,
        .flags = flags,
        .new_thread_id = new_thread_id
    };
	r = syscall1(600, (unsigned long)&args);
	return (int)r.eax;
}

static inline int sys_set_thread_area(struct user_desc *u_info)
{
    int result;

    __asm__ volatile (
        "movl $243, %%eax\n\t"
        "movl %[info], %%ebx\n\t"
        "int $0x80"
        : "=a"(result)
        : [info] "r"(u_info)
        : "ebx", "memory"
    );

    return result;
}

static inline int sys_get_thread_area(struct user_desc *u_info)
{
    int result;

    __asm__ volatile (
        "movl $244, %%eax\n\t"
        "movl %[info], %%ebx\n\t"
        "int $0x80"
        : "=a"(result)
        : [info] "r"(u_info)
        : "ebx", "memory"
    );

    return result;
}

int set_thread_area(struct user_desc *u_info)
{
    return sys_set_thread_area(u_info);
}

int get_thread_area(struct user_desc *u_info)
{
    return sys_get_thread_area(u_info);
}
