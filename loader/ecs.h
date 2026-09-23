// Fast System Kernel Loader - Enhanced Code Section
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef __ECS_H__
#define __ECS_H__

#define ENHANCED_CODE_SECTION __attribute__ ((section (".enhanced_code")))
#define ENHANCED_DATA_SECTION __attribute__ ((section (".enhanced_data")))

#pragma CODE_SECTION (".enhanced_code")
#pragma DATA_SECTION (".enhanced_data")

extern unsigned long enhanced_code;
extern unsigned long enhanced_data;
extern unsigned long code;
extern unsigned long data;

#endif // __ECS_H__
