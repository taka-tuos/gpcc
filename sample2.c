#include "sample.h"
#include "font.h"

void putfont8(unsigned int *vram, int xsize, int x, int y, unsigned int c, unsigned char *font)
{
	int i, j;
	unsigned int *p;
	unsigned char d;
	
	for (i = 0; i < 8; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		for(j = 0; j < 8; j++) if ((d & (0x80 >> j))) { p[j] = c; } else { p[j] = 0; }
	}
	
	return;
}

void putfonts8_asc(int x, int y, int c, char *sz)
{
	char *font;
	unsigned char *s = (unsigned char *)sz;
	int l = 1;

	for (; *s != 0x00; s++) {
		if(*s != 0x0a){
			putfont8((unsigned int *) 0x10000, 640, x, y, c, (unsigned char *)fontx_8x8 + *s * 8);
			l++;
		} else {
			y += 8;
			x -= l * 8;
			l = 1;
		}
		x += 8;
	}
	return;
}

void reflesh(void)
{
	asm("\tout($1,$1)\n");
}
