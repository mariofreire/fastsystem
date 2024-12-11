#!/bin/sh

make fskrnl gen-vars write-vars boot put run_debug | $1 $2 $3 $4 $5
