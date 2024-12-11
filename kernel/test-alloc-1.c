// Fast System Application Test using libc (C Library)
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
	char *str1;
    printf("Allocating 256 bytes to create buffer.\n");
	str1 = (char*)malloc(256);
	printf("Address from allocated buffer 256-bytes: 0x%X\n", (unsigned long)str1);
	strcpy(str1, "Hello World!");
	printf("Print test allocated buffer 256-bytes: %s\n", str1);
	printf("Free allocation of 256 bytes.\n");
	free(str1);
	return 0;
}



