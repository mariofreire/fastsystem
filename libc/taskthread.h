// Fast System Task Thread
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#ifndef __TASK_THREAD_H__
#define __TASK_THREAD_H__

#include <stddef.h>
#include <stdint.h>

#define TASK_UNUSED           0
#define TASK_READY            1
#define TASK_RUNNING          2
#define TASK_SLEEP            3
#define TASK_BLOCK            4
#define TASK_TERMINATE        5

#define MAX_TASKS 65536

#define MAX_PRIORITY   7
#define PRIORITY_LEVELS (MAX_PRIORITY + 1)

#define PRIORITY_IDLE         0
#define PRIORITY_LOW          1
#define PRIORITY_NORMAL       3
#define PRIORITY_HIGH         5
#define PRIORITY_DRIVER       6
#define PRIORITY_CRITICAL     7

#define TASK_THREAD           (0<<1)
#define TASK_PROCESS          (1<<1)

#define PTHREAD_JOIN_SUCCESS       0
#define PTHREAD_JOIN_INVALID      -1
#define PTHREAD_JOIN_DEADLOCK     -2
#define PTHREAD_JOIN_ALREADY      -3
#define PTHREAD_JOIN_NO_JOINER    -4

#define WEXITSTATUS_NOT_FOUND     127

#define TASK_STACK_SIZE 2048

#define STACK_SIZE      4096
#define STACK_WORDS     STACK_SIZE
#define STACK_BYTES     (STACK_WORDS * sizeof(unsigned long))
#define CONTEXT_WORDS (sizeof(context_t) / sizeof(unsigned long))
#define INITIAL_STACK_GUARD_WORDS 1

typedef void *(*thread_entry_t)(void *param);

#pragma pack (push, 1)

typedef struct 
{
    unsigned long gs;
    unsigned long fs;
    unsigned long es;
    unsigned long ds;
    unsigned long edi;
    unsigned long esi;
    unsigned long ebp;
    unsigned long esp;
    unsigned long ebx;
    unsigned long edx;
    unsigned long ecx;
    unsigned long eax;
    unsigned long eip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long useresp;
    unsigned long ss;
} context_t;

typedef struct task
{
	char name[256];
	context_t context;
    context_t *esp;
    uint32_t *stack_top;
    uint32_t stack_size;
    uint32_t kernel_cs;
    uint32_t kernel_ds;
    uint32_t user_cs;
    uint32_t user_ds;
    struct task *parent;
    struct task *prev;
    struct task *next;
    int priority;
    int state;
    int quantum_left;   
    unsigned int wake_tick; 
    unsigned int thread_id;
    thread_entry_t entry;
    void *param;
    void *result;
    void *args;
    int joiner_id;
    int joined;    
    int pid;
    void *module;
    void *start_brk;
    void *brk;
    uint32_t stack[STACK_SIZE];
} task_t;

typedef struct 
{
    task_t *handle_instance;
    const char *name;
    int attributes;
    int stack_size;
    unsigned long *start_address;
    void *param;
    int flags;
    int *new_thread_id;
} createthread_args_t;

#pragma pack (pop)

struct user_desc 
{
    uint32_t entry_number;
    uint32_t base_addr;
    uint32_t limit;
    uint32_t seg_32bit:1;
    uint32_t contents:2;
    uint32_t read_exec_only:1;
    uint32_t limit_in_pages:1;
    uint32_t seg_not_present:1;
    uint32_t useable:1;
    uint32_t lm:1;
};

void task_terminate(unsigned long task, unsigned long signal);
int get_task_priority(int type, unsigned long task);
int set_task_priority(int type, unsigned long task, unsigned long priority);
int pthread_join(unsigned long thread, void **result);

int createthread(task_t *handle_instance,
                 const char *name,
                 int attributes,
                 int stack_size,
                 unsigned long *start_address,
                 void *param,
                 int flags,
                 int *new_thread_id);
                 
#endif // __TASK_THREAD_H__
