#!/bin/sh

if [ "$1" = "" ]; then
	echo "Memory Hex Dump         by   Mario Freire"
	echo "Copyright (C) 2024 DSP Interactive."
	echo ""
	echo "Usage:"
	echo "mhexdump [memory-address]"
	echo ""
	echo "Example:"
	echo "mhexdump 0xB8000"
	echo ""
else
	sudo hexdump -C -n 256 -s $1 /dev/mem
fi
