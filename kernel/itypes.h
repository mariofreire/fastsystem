// Fast System Int Types
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __ITYPES_H__
#define __ITYPES_H__

#include "float.h"

#define NULL 0LL

#define TRUE 1
#define FALSE 0
#define true TRUE
#define false FALSE

typedef long unsigned int size_t;

typedef unsigned char bool;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

#define FLAGBIT(value, index) ((value >> index) & ((1 << 1) - 1))

#define UCHAR8A(value) ((unsigned char)(value))
#define UCHAR8B(value) ((unsigned char)((value)>> 8))
#define UCHAR8C(value) ((unsigned char)((value)>>16))
#define UCHAR8D(value) ((unsigned char)((value)>>24))
#define UINT16(a,b) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)))
#define UINT32(a,b,c,d) ((unsigned long)((unsigned char)(a)|((unsigned char)(b)<<8)|((unsigned char)(c)<<16)|((unsigned char)(d)<<24)))
#define USHORT16(a,b) ((unsigned long)(((unsigned long)(a)<<16)|((unsigned short)(b))))

#define HIGH16(a) ((unsigned short)(((a)>>16)&0xFFFF))
#define LOW16(a) ((unsigned short)((a)&0xFFFF))

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;

#endif // __ITYPES_H__
