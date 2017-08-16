#include "sample.h"
#include "xprintf.h"

int main(void)
{
	char s[30];
	int n = 114514;
	xsprintf(s,"The C++%d TOO LATE Programming Language",n);
	
	putfonts8_asc(2,2,0xffffff,s);
	
	reflesh();
	while(1);
}
