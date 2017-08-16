#!/bin/sh
./as/as crt0.s crt0.obj -coff
./cc -I golibc/ -coff sample.c sample.obj
./cc -I golibc/ -coff sample2.c sample2.obj
./cc -I golibc/ -coff xprintf.c xprintf.obj
./ld/ld gios.bin 0x00310000 crt0.obj sample.obj sample2.obj xprintf.obj
