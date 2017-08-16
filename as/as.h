#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct {
	char type;
	int64_t num;
	int size;
} args_t;

typedef struct {
	char bin[256];
	int bytes;
} binary_t;

typedef struct {
	char name[64];
	int address;
} label_t;

typedef struct {
	char name[64];
	unsigned int attribute[16];
} symbol_t;

#define		ARGS_IR		(0x00)
#define		ARGS_MR		(0x01)
#define		ARGS_ME		(0x02)
#define		ARGS_E		(0x03)
#define		ARGS_FR		(0x04)
#define		ARGS_FE		(0x05)
#define		ARGS_SR		(0x06)
#define		ARGS_MRAE	(0x07)
#define		ARGS_MRAR	(0x08)
#define		ARGS_RAE	(0x09)
#define		ARGS_RAR	(0x0a)

#ifndef UINT
typedef unsigned int UINT;
#endif
#ifndef INT16
typedef int16_t INT16;
#endif
#ifndef INT32
typedef int32_t INT32;
#endif
#ifndef UINT32
typedef uint32_t UINT32;
#endif
#ifndef UINT64
typedef uint64_t UINT64;
#endif

typedef UINT32 gp3200_float;

gp3200_float gp3200_f32_float2f32(double val);
gp3200_float gp3200_f32_int2float(UINT32 val);
gp3200_float gp3200_f32_fadd(gp3200_float val1, gp3200_float val2);
gp3200_float gp3200_f32_fsub(gp3200_float val1, gp3200_float val2);
gp3200_float gp3200_f32_fmul(gp3200_float val1, gp3200_float val2);
gp3200_float gp3200_f32_fdiv(gp3200_float val1, gp3200_float val2);
double gp3200_f32_getfloat(gp3200_float float32);
