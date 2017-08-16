TARGET		= cc
OBJS_TARGET	= cc.o

CFLAGS = -O0 -g
LIBS = 

include Makefile.in

sample : sample.c sample2.c sample.h font.h Makefile Makefile.in
	 rm *.tmp 
	 as/as crt0.s crt0.o -coff
	 ./cc -I golibc sample.c sample.o -coff
	 ./cc -I golibc sample2.c sample2.o -coff
	 ./cc -I golibc xprintf.c xprintf.o -coff
	 ld/ld gios.bin 0x00310000 crt0.o sample.o sample2.o xprintf.o
