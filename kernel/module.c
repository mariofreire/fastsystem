// Fast System Kernel Module Loader
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2026 DSP Interactive.

#include "itypes.h"
#include "errno.h"
#include "string.h"
#include "module.h"
#include "vfs.h"

void printk(const char *msg, ...);
void *malloc(size_t size);
void free(void *ptr);

extern unsigned char fileexists(const char *filename);
extern unsigned long getfilesize(const char *filename);
extern unsigned char getfiledata(const char *filename, unsigned char *data);

#define EI_NIDENT 16
#define ELFCLASS32 1
#define ELFDATA2LSB 1
#define ET_REL 1
#define EM_386 3

#define SHT_NULL     0
#define SHT_PROGBITS 1
#define SHT_SYMTAB   2
#define SHT_STRTAB   3
#define SHT_RELA     4
#define SHT_NOBITS   8
#define SHT_REL      9

#define SHF_ALLOC 0x2

#define SHN_UNDEF  0
#define SHN_ABS    0xfff1
#define SHN_COMMON 0xfff2

#define STB_LOCAL  0
#define STB_GLOBAL 1
#define STB_WEAK   2

#define R_386_NONE  0
#define R_386_32    1
#define R_386_PC32  2
#define R_386_PLT32 4

#define ELF32_ST_BIND(i) ((unsigned char)((i) >> 4))
#define ELF32_R_SYM(i)   ((unsigned long)(i) >> 8)
#define ELF32_R_TYPE(i)  ((unsigned char)(i))

#define MODULE_IMAGE_MAX (8UL * 1024UL * 1024UL)

typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned long e_version;
    unsigned long e_entry;
    unsigned long e_phoff;
    unsigned long e_shoff;
    unsigned long e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} module_ehdr_t;

typedef struct
{
    unsigned long sh_name;
    unsigned long sh_type;
    unsigned long sh_flags;
    unsigned long sh_addr;
    unsigned long sh_offset;
    unsigned long sh_size;
    unsigned long sh_link;
    unsigned long sh_info;
    unsigned long sh_addralign;
    unsigned long sh_entsize;
} module_shdr_t;

typedef struct
{
    unsigned long st_name;
    unsigned long st_value;
    unsigned long st_size;
    unsigned char st_info;
    unsigned char st_other;
    unsigned short st_shndx;
} module_sym_t;

typedef struct
{
    unsigned long r_offset;
    unsigned long r_info;
} module_rel_t;

typedef struct
{
    const char *name;
    unsigned long value;
} module_export_t;

static kernel_module_t module_table[MODULE_MAX];
static int module_ready = 0;

static const module_export_t module_exports[] =
{
    { "printk", (unsigned long)printk },
    { "malloc", (unsigned long)malloc },
    { "free", (unsigned long)free },
    { "memcpy", (unsigned long)memcpy },
    { "memset", (unsigned long)memset },
    { "memmove", (unsigned long)memmove },
    { "strcpy", (unsigned long)strcpy },
    { "strcmp", (unsigned long)strcmp },
    { "strlen", (unsigned long)strlen },
    { "strncpy", (unsigned long)strncpy },
    { "strncmp", (unsigned long)strncmp },
    { "vfs_register", (unsigned long)vfs_register },
    { "vfs_mount", (unsigned long)vfs_mount },
    { "vfs_umount", (unsigned long)vfs_umount },
    { "vfs_find_fs", (unsigned long)vfs_find_fs },
    { "fileexists", (unsigned long)fileexists },
    { "getfilesize", (unsigned long)getfilesize },
    { "getfiledata", (unsigned long)getfiledata },
    { NULL, 0 }
};

static void module_copy(char *dst, const char *src, size_t n)
{
    size_t i;

    if (n == 0)
        return;
    if (src == NULL)
        src = "";
    for (i = 0; src[i] != 0 && (i + 1) < n; i++)
        dst[i] = src[i];
    dst[i] = 0;
}

static unsigned long module_read32(const void *p)
{
    unsigned long v;
    memcpy(&v, p, 4);
    return v;
}

static void module_write32(void *p, unsigned long v)
{
    memcpy(p, &v, 4);
}

static int module_in_range(const unsigned char *image, unsigned long size, unsigned long off, unsigned long len)
{
    (void)image;
    if (len > size)
        return 0;
    if (off > size - len)
        return 0;
    return 1;
}

static const module_shdr_t *module_shdr(const unsigned char *image, const module_ehdr_t *eh, unsigned int index)
{
    return (const module_shdr_t *)(image + eh->e_shoff + (unsigned long)index * eh->e_shentsize);
}

static unsigned long module_align(unsigned long value)
{
    if (value < 2)
        return 1;
    if ((value & (value - 1)) != 0)
        return 4;
    if (value > 4096)
        return 4096;
    return value;
}

static unsigned long module_lookup_export(const char *name)
{
    int i;

    if (name == NULL || name[0] == 0)
        return 0;
    for (i = 0; module_exports[i].name != NULL; i++)
    {
        if (strcmp(module_exports[i].name, name) == 0)
            return module_exports[i].value;
    }
    return 0;
}

static void module_release(kernel_module_t *mod)
{
    int i;

    if (mod == NULL)
        return;
    for (i = 0; i < mod->section_count && i < MODULE_MAX_SECTIONS; i++)
    {
        if (mod->section[i] != NULL)
            free(mod->section[i]);
        mod->section[i] = NULL;
    }
    mod->section_count = 0;
    mod->init = NULL;
    mod->exit = NULL;
    mod->used = 0;
}

static const char *module_basename(const char *path)
{
    const char *base;
    const char *s;

    base = path != NULL ? path : "";
    for (s = base; *s != 0; s++)
    {
        if (*s == '/' || *s == '\\')
            base = s + 1;
    }
    return base;
}

static void module_name_from_path(const char *path, char *name, size_t name_sz)
{
    const char *base;
    size_t n;
    size_t i;

    base = module_basename(path);
    n = strlen(base);
    if (n > 2 && base[n - 2] == '.' && (base[n - 1] == 'o' || base[n - 1] == 'O'))
        n -= 2;
    if (n == 0)
    {
        module_copy(name, "module", name_sz);
        return;
    }
    if (n >= name_sz)
        n = name_sz - 1;
    for (i = 0; i < n; i++)
        name[i] = base[i];
    name[n] = 0;
}

static int module_pointer_in(unsigned long ptr, unsigned long *base, unsigned long *size, unsigned int count)
{
    unsigned int i;

    for (i = 0; i < count; i++)
    {
        if (base[i] == 0 || size[i] == 0)
            continue;
        if (ptr >= base[i] && ptr < base[i] + size[i])
            return 1;
    }
    return 0;
}

int module_loader_init(void)
{
    memset(module_table, 0, sizeof(module_table));
    module_ready = 1;
    return 0;
}

kernel_module_t *module_find(const char *name)
{
    int i;

    if (name == NULL || name[0] == 0)
        return NULL;
    for (i = 0; i < MODULE_MAX; i++)
    {
        if (!module_table[i].used)
            continue;
        if (strcmp(module_table[i].name, name) == 0)
            return &module_table[i];
    }
    return NULL;
}

kernel_module_t *module_get(int index)
{
    if (index < 0 || index >= MODULE_MAX)
        return NULL;
    return &module_table[index];
}

int module_load_image(const char *path, const void *image_ptr, unsigned long size)
{
    const unsigned char *image;
    const module_ehdr_t *eh;
    kernel_module_t loaded;
    unsigned long *base;
    unsigned long *sec_size;
    void **raw;
    unsigned long *symval;
    int stored;
    unsigned int s;
    int rc;
    int slot;
    const char *name_sym;

    if (!module_ready)
        return -ENODEV;
    if (image_ptr == NULL || size < sizeof(module_ehdr_t))
        return -ENOEXEC;
    if (size > MODULE_IMAGE_MAX)
        return -EFBIG;

    image = (const unsigned char *)image_ptr;
    eh = (const module_ehdr_t *)image;

    if (eh->e_ident[0] != 0x7f ||
        eh->e_ident[1] != 'E' ||
        eh->e_ident[2] != 'L' ||
        eh->e_ident[3] != 'F')
        return -ENOEXEC;
    if (eh->e_ident[4] != ELFCLASS32 || eh->e_ident[5] != ELFDATA2LSB)
        return -ENOEXEC;
    if (eh->e_type != ET_REL || eh->e_machine != EM_386)
        return -ENOEXEC;
    if (eh->e_shoff == 0 || eh->e_shnum == 0 || eh->e_shentsize < sizeof(module_shdr_t))
        return -ENOEXEC;
    if (eh->e_shnum > 256)
        return -ENOEXEC;
    if (!module_in_range(image, size, eh->e_shoff, (unsigned long)eh->e_shnum * eh->e_shentsize))
        return -ENOEXEC;

    memset(&loaded, 0, sizeof(loaded));
    module_name_from_path(path, loaded.name, sizeof(loaded.name));
    module_copy(loaded.path, path != NULL ? path : "", sizeof(loaded.path));

    base = (unsigned long *)malloc(eh->e_shnum * sizeof(unsigned long));
    sec_size = (unsigned long *)malloc(eh->e_shnum * sizeof(unsigned long));
    raw = (void **)malloc(eh->e_shnum * sizeof(void *));
    if (base == NULL || sec_size == NULL || raw == NULL)
    {
        free(base);
        free(sec_size);
        free(raw);
        return -ENOMEM;
    }
    memset(base, 0, eh->e_shnum * sizeof(unsigned long));
    memset(sec_size, 0, eh->e_shnum * sizeof(unsigned long));
    memset(raw, 0, eh->e_shnum * sizeof(void *));

    stored = 0;
    rc = 0;

    for (s = 0; s < eh->e_shnum; s++)
    {
        const module_shdr_t *sh;
        unsigned long align;
        unsigned long alloc_size;
        unsigned char *mem;
        unsigned long addr;
        unsigned long aligned;

        sh = module_shdr(image, eh, s);
        if ((sh->sh_flags & SHF_ALLOC) == 0)
            continue;
        if (sh->sh_type != SHT_PROGBITS && sh->sh_type != SHT_NOBITS)
            continue;
        if (sh->sh_size == 0)
            continue;
        if (sh->sh_size > MODULE_IMAGE_MAX)
        {
            rc = -EFBIG;
            break;
        }
        if (stored >= MODULE_MAX_SECTIONS)
        {
            rc = -ENOMEM;
            break;
        }
        if (sh->sh_type == SHT_PROGBITS &&
            !module_in_range(image, size, sh->sh_offset, sh->sh_size))
        {
            rc = -ENOEXEC;
            break;
        }

        align = module_align(sh->sh_addralign);
        alloc_size = sh->sh_size + align;
        mem = (unsigned char *)malloc(alloc_size);
        if (mem == NULL)
        {
            rc = -ENOMEM;
            break;
        }
        memset(mem, 0, alloc_size);
        addr = (unsigned long)mem;
        aligned = (addr + align - 1) & ~(align - 1);
        if (sh->sh_type == SHT_PROGBITS)
            memcpy((void *)aligned, image + sh->sh_offset, sh->sh_size);

        raw[s] = mem;
        base[s] = aligned;
        sec_size[s] = sh->sh_size;
        loaded.section[stored++] = mem;
    }
    loaded.section_count = stored;

    if (rc != 0)
    {
        free(base);
        free(sec_size);
        free(raw);
        module_release(&loaded);
        return rc;
    }

    symval = NULL;
    name_sym = NULL;

    {
        const module_shdr_t *symsh = NULL;
        const module_shdr_t *strsh;
        const unsigned char *strtab;
        unsigned long symcount;
        unsigned long i;

        for (s = 0; s < eh->e_shnum; s++)
        {
            const module_shdr_t *sh = module_shdr(image, eh, s);
            if (sh->sh_type == SHT_SYMTAB)
            {
                symsh = sh;
                break;
            }
        }
        if (symsh == NULL || symsh->sh_link >= eh->e_shnum || symsh->sh_entsize < sizeof(module_sym_t))
            rc = -ENOEXEC;
        else if (!module_in_range(image, size, symsh->sh_offset, symsh->sh_size))
            rc = -ENOEXEC;

        if (rc == 0)
        {
            strsh = module_shdr(image, eh, (unsigned int)symsh->sh_link);
            if (!module_in_range(image, size, strsh->sh_offset, strsh->sh_size) || strsh->sh_size == 0)
                rc = -ENOEXEC;
            else
                strtab = image + strsh->sh_offset;
        }

        if (rc == 0)
        {
            symcount = symsh->sh_size / symsh->sh_entsize;
            if (symcount == 0 || symcount > 100000UL)
                rc = -ENOEXEC;
            else
            {
                symval = (unsigned long *)malloc(symcount * sizeof(unsigned long));
                if (symval == NULL)
                    rc = -ENOMEM;
                else
                    memset(symval, 0, symcount * sizeof(unsigned long));
            }
        }

        for (i = 0; rc == 0 && i < symcount; i++)
        {
            const module_sym_t *sym;
            const char *sname;
            unsigned long value;
            unsigned char bind;

            sym = (const module_sym_t *)(image + symsh->sh_offset + i * symsh->sh_entsize);
            sname = "";
            if (sym->st_name != 0)
            {
                if (sym->st_name >= strsh->sh_size)
                {
                    rc = -ENOEXEC;
                    break;
                }
                sname = (const char *)(strtab + sym->st_name);
            }
            bind = ELF32_ST_BIND(sym->st_info);
            value = 0;

            if (sym->st_shndx == SHN_UNDEF)
            {
                value = module_lookup_export(sname);
                if (value == 0 && bind != STB_WEAK && sname[0] != 0)
                {
                    printk("module: undefined symbol %s\n", sname);
                    rc = -ENOEXEC;
                    break;
                }
            }
            else if (sym->st_shndx == SHN_ABS)
            {
                value = sym->st_value;
            }
            else if (sym->st_shndx == SHN_COMMON)
            {
                unsigned long align;
                unsigned long alloc_size;
                unsigned char *mem;
                unsigned long addr;
                unsigned long aligned;

                if (sym->st_size == 0 || loaded.section_count >= MODULE_MAX_SECTIONS)
                {
                    rc = -ENOMEM;
                    break;
                }
                align = module_align(sym->st_value);
                alloc_size = sym->st_size + align;
                mem = (unsigned char *)malloc(alloc_size);
                if (mem == NULL)
                {
                    rc = -ENOMEM;
                    break;
                }
                memset(mem, 0, alloc_size);
                addr = (unsigned long)mem;
                aligned = (addr + align - 1) & ~(align - 1);
                loaded.section[loaded.section_count++] = mem;
                value = aligned;
            }
            else if (sym->st_shndx < eh->e_shnum)
            {
                if (base[sym->st_shndx] == 0)
                    value = sym->st_value;
                else
                    value = base[sym->st_shndx] + sym->st_value;
            }
            else
            {
                rc = -ENOEXEC;
                break;
            }

            symval[i] = value;

            if (sname[0] != 0 && (bind == STB_GLOBAL || bind == STB_WEAK))
            {
                if (strcmp(sname, "module_init") == 0 && value != 0)
                    loaded.init = (module_init_fn)value;
                else if (strcmp(sname, "module_exit") == 0 && value != 0)
                    loaded.exit = (module_exit_fn)value;
                else if (strcmp(sname, "module_name") == 0 && value != 0)
                    name_sym = (const char *)value;
            }
        }

        for (s = 0; rc == 0 && s < eh->e_shnum; s++)
        {
            const module_shdr_t *sh;
            const module_shdr_t *linksh;
            unsigned long count;
            unsigned long r;

            sh = module_shdr(image, eh, s);
            if (sh->sh_type != SHT_REL)
                continue;
            if (sh->sh_entsize < sizeof(module_rel_t) || sh->sh_size == 0)
                continue;
            if (sh->sh_info >= eh->e_shnum || sh->sh_link >= eh->e_shnum)
            {
                rc = -ENOEXEC;
                break;
            }
            linksh = module_shdr(image, eh, (unsigned int)sh->sh_link);
            if (linksh->sh_type != SHT_SYMTAB)
            {
                rc = -ENOEXEC;
                break;
            }
            if ((module_shdr(image, eh, (unsigned int)sh->sh_info)->sh_flags & SHF_ALLOC) == 0)
                continue;
            if (base[sh->sh_info] == 0)
                continue;
            if (!module_in_range(image, size, sh->sh_offset, sh->sh_size))
            {
                rc = -ENOEXEC;
                break;
            }

            count = sh->sh_size / sh->sh_entsize;
            for (r = 0; r < count; r++)
            {
                const module_rel_t *rel;
                unsigned long sym_index;
                unsigned long type;
                unsigned long S;
                unsigned long A;
                unsigned long P;
                unsigned char *loc;

                rel = (const module_rel_t *)(image + sh->sh_offset + r * sh->sh_entsize);
                sym_index = ELF32_R_SYM(rel->r_info);
                type = ELF32_R_TYPE(rel->r_info);
                if (sym_index >= symcount)
                {
                    rc = -ENOEXEC;
                    break;
                }
                if (type != R_386_NONE && rel->r_offset + 4 > sec_size[sh->sh_info])
                {
                    rc = -ENOEXEC;
                    break;
                }
                loc = (unsigned char *)(base[sh->sh_info] + rel->r_offset);
                P = (unsigned long)loc;
                S = symval[sym_index];
                A = module_read32(loc);

                if (type == R_386_NONE)
                    continue;
                if (type == R_386_32)
                {
                    module_write32(loc, S + A);
                    continue;
                }
                if (type == R_386_PC32 || type == R_386_PLT32)
                {
                    module_write32(loc, S + A - P);
                    continue;
                }

                printk("module: unsupported relocation %d\n", (int)type);
                rc = -ENOEXEC;
                break;
            }
        }
    }
    if (rc == 0 && loaded.init == NULL)
    {
        printk("module: missing module_init\n");
        rc = -ENOEXEC;
    }
    if (rc == 0 && !module_pointer_in((unsigned long)loaded.init, base, sec_size, eh->e_shnum))
    {
        printk("module: module_init is not inside the module\n");
        rc = -ENOEXEC;
    }
    if (rc == 0 && loaded.exit != NULL &&
        !module_pointer_in((unsigned long)loaded.exit, base, sec_size, eh->e_shnum))
    {
        printk("module: module_exit is not inside the module\n");
        rc = -ENOEXEC;
    }

    if (rc == 0 && name_sym != NULL && name_sym[0] != 0)
        module_copy(loaded.name, name_sym, sizeof(loaded.name));

    free(base);
    free(sec_size);
    free(raw);
    free(symval);
    base = NULL;
    sec_size = NULL;
    raw = NULL;
    symval = NULL;

    if (rc != 0)
    {
        module_release(&loaded);
        return rc;
    }

    if (module_find(loaded.name) != NULL)
    {
        module_release(&loaded);
        return -EEXIST;
    }

    slot = -1;
    {
        int i;
        for (i = 0; i < MODULE_MAX; i++)
        {
            if (!module_table[i].used)
            {
                slot = i;
                break;
            }
        }
    }
    if (slot < 0)
    {
        module_release(&loaded);
        return -ENOMEM;
    }

    rc = loaded.init();
    if (rc != 0)
    {
        module_release(&loaded);
        if (rc > 0)
            return -rc;
        return rc;
    }

    loaded.used = 1;
    module_table[slot] = loaded;
    printk("module: %s loaded\n", loaded.name);
    return 0;
}

int module_load(const char *path)
{
    unsigned long size;
    unsigned char *image;
    int rc;

    if (!module_ready)
        return -ENODEV;
    if (path == NULL || path[0] == 0)
        return -EINVAL;
    if (!fileexists(path))
        return -ENOENT;

    size = getfilesize(path);
    if (size < sizeof(module_ehdr_t) || size > MODULE_IMAGE_MAX)
        return -ENOEXEC;

    image = (unsigned char *)malloc(size);
    if (image == NULL)
        return -ENOMEM;
    if (!getfiledata(path, image))
    {
        free(image);
        return -EIO;
    }

    rc = module_load_image(path, image, size);
    free(image);
    return rc;
}

int module_unload(const char *name)
{
    kernel_module_t *mod;
    int rc;

    if (!module_ready)
        return -ENODEV;
    mod = module_find(name);
    if (mod == NULL)
        return -ENOENT;

    if (mod->exit != NULL)
    {
        rc = mod->exit();
        if (rc != 0)
            return rc > 0 ? -rc : rc;
    }

    printk("module: %s unloaded\n", mod->name);
    module_release(mod);
    memset(mod, 0, sizeof(*mod));
    return 0;
}
