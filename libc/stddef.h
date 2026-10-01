// Fast System Standard Define
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef _STDDEF_H
#define _STDDEF_H

typedef unsigned int size_t;
typedef int ptrdiff_t;

#define NULL ((void *)0)

#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#endif
