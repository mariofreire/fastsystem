// Fast System Kernel Module Loader
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.
//
// Modules are ELF32 relocatable objects (ET_REL, EM_386), the same
// objects gcc -c produces. Build them with the kernel CFLAGS
// (-m32 -ffreestanding -fno-pic -fno-pie, no stack protector).
//
// Required symbol:  int module_init(void);
// Optional symbol:  int module_exit(void);
// Optional symbol:  const char module_name[];
//
// Undefined symbols are bound to the kernel export table
// (printk, malloc, free, the VFS register/mount calls, ...).

#ifndef __MODULE_H__
#define __MODULE_H__

#include "itypes.h"

#define MODULE_NAME_MAX      32
#define MODULE_PATH_MAX      256
#define MODULE_MAX           16
#define MODULE_MAX_SECTIONS  64

typedef int (*module_init_fn)(void);
typedef int (*module_exit_fn)(void);

typedef struct kernel_module
{
    char name[MODULE_NAME_MAX];
    char path[MODULE_PATH_MAX];
    module_init_fn init;
    module_exit_fn exit;
    void *section[MODULE_MAX_SECTIONS];
    int section_count;
    int used;
} kernel_module_t;

int module_loader_init(void);
int module_load(const char *path);
int module_load_image(const char *path, const void *image, unsigned long size);
int module_unload(const char *name);
kernel_module_t *module_find(const char *name);
kernel_module_t *module_get(int index);

#endif /* __MODULE_H__ */
