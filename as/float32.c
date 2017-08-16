/*
	GP3200 EMU float32
	cording by taka
*/
#include "as.h"


gp3200_float gp3200_f32_int2float(UINT32 val)
{
	return val * 1024;
}

gp3200_float gp3200_f32_float2f32(double val)
{
	return val * 1024;
}

gp3200_float gp3200_f32_fadd(gp3200_float val1, gp3200_float val2)
{
	return gp3200_f32_float2f32(gp3200_f32_getfloat(val1) + gp3200_f32_getfloat(val2));
}

gp3200_float gp3200_f32_fsub(gp3200_float val1, gp3200_float val2)
{
	return gp3200_f32_float2f32(gp3200_f32_getfloat(val1) - gp3200_f32_getfloat(val2));
}

gp3200_float gp3200_f32_fmul(gp3200_float val1, gp3200_float val2)
{
	return gp3200_f32_float2f32(gp3200_f32_getfloat(val1) * gp3200_f32_getfloat(val2));
}

gp3200_float gp3200_f32_fdiv(gp3200_float val1, gp3200_float val2)
{
	return gp3200_f32_float2f32(gp3200_f32_getfloat(val1) / gp3200_f32_getfloat(val2));
}

double gp3200_f32_getfloat(gp3200_float float32)
{
	return (double)float32 / (double)1024;
}