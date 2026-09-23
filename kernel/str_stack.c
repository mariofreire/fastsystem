#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MODULE_STACK_SIZE 4096
#define MODULE_NAME_SIZE 256

char **module_stack;
int module_stack_top=-1;


unsigned char is_module_stack_empty(void)
{
    if (module_stack_top == -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char is_module_stack_full(void)
{
    if (module_stack_top >= MODULE_STACK_SIZE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

char* get_current_module(void)
{
    if (module_stack_top == -1)
    {
        return NULL;
    }
    else if (module_stack_top >= MODULE_STACK_SIZE)
    {
        return NULL;
    }
    else
    {
        return module_stack[module_stack_top];
    }
}

char * push_module(const char *module_name)
{
    if (!is_module_stack_full())
    {
        module_stack_top = module_stack_top + 1;
        strcpy(module_stack[module_stack_top], module_name);
        return module_stack[module_stack_top];
    }
    else
    {
        printf("Stack Overflow\n");
    }
    return NULL;
}

char *pop_module(void)
{
    char *module_data;
    if (!is_module_stack_empty())
    {
        module_data = module_stack[module_stack_top];
        module_stack_top = module_stack_top - 1;
        return module_data;
    }
    else
    {
        printf("Stack Underflow\n");
    }
    return NULL;
}

void init_module_stack(void)
{
    module_stack = (char**)malloc(MODULE_STACK_SIZE*MODULE_NAME_SIZE);
    for(int i=0;i<MODULE_STACK_SIZE;i++)
    {
        module_stack[i] = (char*)malloc(MODULE_NAME_SIZE);
    }
    
}

void uninit_module_stack(void)
{
    for(int i=0;i<MODULE_STACK_SIZE;i++)
    {
        free(module_stack[i]);
    }
    free(module_stack);
}

int main()
{
    init_module_stack();
    
    push_module("first");
    push_module("second");
    push_module("third");
    pop_module();
    printf("%s\n", get_current_module());
    pop_module();
    push_module("fourth");
    printf("%s\n", get_current_module());
    pop_module();
    
    uninit_module_stack();
    return 0;
}
