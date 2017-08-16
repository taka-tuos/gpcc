#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INVALID		-0x7fffffff

typedef struct {
	char name[64];
	unsigned int attribute[16];
} symbol_t __attribute__((packed));

typedef struct {
	char sig[4];
	int entno;
	int repaddr;
	int repno;
	symbol_t symbols[];
} coffhead_t __attribute__((packed));

int base_address = 0;

char *image[512];
char *imagename[512];
int imagesize[512];
int readed_n = 0;

typedef struct {
	int img,idx;
} symbol_ptr;

char *skipspace(char *p)
{
	for (; *p == ' '; p++) { }	/* スペースを読み飛ばす */
	return p;
}

int get_immediate_sub(char **pp, int priority)
{
	char *p = *pp;
	int i = INVALID, j;
	p = skipspace(p);

	/* 単項演算子 */
	if (*p == '+') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 0);
	} else if (*p == '-') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 0);
		if (i != INVALID) {
			i = - i;
		}
	} else if (*p == '~') {
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 0);
		if (i != INVALID) {
			i = ~i;
		}
	} else if (*p == '(') {	/* かっこ */
		p = skipspace(p + 1);
		i = get_immediate_sub(&p, 9);
		if (*p == ')') {
			p = skipspace(p + 1);
		} else {
			i = INVALID;
		}
	} else if ('0' <= *p && *p <= '9') { /* 数値 */
		i = strtol(p, &p, 0);
	} else { /* エラー */
		i = INVALID;
	}

	/* 二項演算子 */
	for (;;) {
		if (i == INVALID) {
			break;
		}
		p = skipspace(p);
		if (*p == '+' && priority > 2) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 2);
			if (j != INVALID) {
				i += j;
			} else {
				i = INVALID;
			}
		} else if (*p == '-' && priority > 2) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 2);
			if (j != INVALID) {
				i -= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '*' && priority > 1) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 1);
			if (j != INVALID) {
				i *= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '/' && priority > 1) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 1);
			if (j != INVALID && j != 0) {
				i /= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '%' && priority > 1) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 1);
			if (j != INVALID && j != 0) {
				i %= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '<' && p[1] == '<' && priority > 3) {
			p = skipspace(p + 2);
			j = get_immediate_sub(&p, 3);
			if (j != INVALID && j != 0) {
				i <<= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '>' && p[1] == '>' && priority > 3) {
			p = skipspace(p + 2);
			j = get_immediate_sub(&p, 3);
			if (j != INVALID && j != 0) {
				i >>= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '&' && priority > 4) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 4);
			if (j != INVALID) {
				i &= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '^' && priority > 5) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 5);
			if (j != INVALID) {
				i ^= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '|' && priority > 6) {
			p = skipspace(p + 1);
			j = get_immediate_sub(&p, 6);
			if (j != INVALID) {
				i |= j;
			} else {
				i = INVALID;
			}
		} else {
			break;
		}
	}
	p = skipspace(p);
	*pp = p;
	return i;
}

int get_immediate(char *s)
{
	char *p = (char *)malloc(1024);
	strcpy(p,s);
	
	return get_immediate_sub(&p,10);
}

symbol_t symbol_find(char *sz, int *img)
{
	int i, j;
	symbol_t r;
	
	memset(&r,0,sizeof(r));
	
	for(i = 0; i < readed_n; i++) {
		coffhead_t *p = (coffhead_t *)image[i];
		if(strncmp(p->sig,"GP32",4) == 0) {
			int j;
			for(j = 0; j < p->entno; j++) {
				if(strcmp(p->symbols[j].name,sz) == 0 && p->symbols[j].attribute[0] < 0x40000000) {
					*img = i;
					return p->symbols[j];
				}
			}
		}
	}
	
	return r;
}

symbol_t symbol_find_head(coffhead_t *p, char *sz)
{
	int j;
	symbol_t r;
	
	memset(&r,0,sizeof(r));
	
	if(strncmp(p->sig,"GP32",4) == 0) {
		int j;
		for(j = 0; j < p->entno; j++) {
			if(strcmp(p->symbols[j].name,sz) == 0 && p->symbols[j].attribute[0] < 0x40000000) {
				return p->symbols[j];
			}
		}
	}
	return r;
}

int symbol_rep_find(coffhead_t *p, int d)
{
	int i;
	
	for(i = 0; i < p->entno; i++) {
		if(p->symbols[i].attribute[0] == d) return i;
	}
	
	return -1;
}

char *symbol_attr_find(coffhead_t *p, int i)
{
	char *w = (char *)malloc(512);
	
	strcpy(w,p->symbols[i].name);
	
	return w;
}

int get_remoteness(int img)
{
	int i, j;
	for(i = 0, j = 0; i < img; i++) {
		coffhead_t *p = (coffhead_t *)image[i];
		j += imagesize[i]-(p->repaddr + p->repno * 4);
	}
	
	return j;
}

unsigned int rep_entry(coffhead_t *p, int img, int idx)
{
	unsigned int *rep = (unsigned int *)(image[img] + p->repaddr);
	return *((unsigned int *)&image[img][rep[idx]+p->repaddr+p->repno*4]);
}

void rep_entry_write(coffhead_t *p, int img, int idx, unsigned int data)
{
	int *rep = (int *)(image[img] + p->repaddr);
	*((unsigned int *)&image[img][rep[idx]+p->repaddr+p->repno*4]) = data;
}

int main(int argc, char *argv[])
{
	if(argc < 5) {
		puts("too few argments");
		printf("usage : %s <outname> <origin> <startup> <object file> ...", argv[0]);
		return 1;
	}
	
	base_address = get_immediate(argv[2]);
	
	int i;
	for(i = 3; i < argc; i++) {
		image[readed_n] = (char *)malloc(16777216);
		imagename[readed_n] = argv[i];
		FILE *fp = fopen(argv[i],"rb");
		if(!fp) {
			printf("unable to find object file %s",argv[i]);
			return 2;
		}
		imagesize[readed_n] = fread(image[readed_n],1,16777216,fp);
		fclose(fp);
		readed_n++;
	}
	
	for(i = 0; i < readed_n; i++) {
		coffhead_t *p = (coffhead_t *)image[i];
		if(strncmp(p->sig,"GP32",4) == 0) {
			int j;
			for(j = 0; j < p->entno; j++) {
				printf("name : %s\nattr[0] : %08x\n\n",p->symbols[j].name,p->symbols[j].attribute[0]);
			}
			int *rep = (int *)(image[i] + p->repaddr);
			for(j = 0; j < p->repno; j++) {
				printf("rep[%d] %08x(%08x)\n",j,rep_entry(p,i,j),rep[j]);
			}
		}
		puts("");
	}
	
	coffhead_t *f = (coffhead_t *)malloc(1048576);
	memset(f,0,1048576);
	int fimg[65536];
	memset(fimg,0,65536*4);
	
	for(i = 0; i < readed_n; i++) {
		coffhead_t *p = (coffhead_t *)image[i];
		if(strncmp(p->sig,"GP32",4) == 0) {
			int j;
			for(j = 0; j < p->repno; j++) {
				int img;
				int d = rep_entry(p,i,j);
				int attr = symbol_rep_find(p,d);
				if(attr == -1) {
					continue;
				}
				char *name = symbol_attr_find(p,attr);
				printf(" repdata : %08x\n attr : %08x\n name : %s\n",d,attr,name);
				symbol_t t = symbol_find(name,&img);
				int k = f->entno;
				f->symbols[k] = t;
				fimg[k] = img;
				f->entno++;
			}
		}
		puts("");
	}
	
	int j;
	for(j = 0; j < f->entno; j++) {
		printf("img : %d\nname : %s\nattr[0] : %08x\n\n",fimg[j],f->symbols[j].name,f->symbols[j].attribute[0]);
	}
	
	int remoteness = 0;
	
	for(i = 0; i < readed_n; i++) {
		coffhead_t *p = (coffhead_t *)image[i];
		if(strncmp(p->sig,"GP32",4) == 0) {
			int *rep = (int *)(image[i] + p->repaddr);
			for(j = 0; j < p->repno; j++) {
				unsigned int repd = rep_entry(p,i,j);
				int attr = symbol_rep_find(p,repd);
				if(attr == -1) {
					rep_entry_write(p,i,j,repd+get_remoteness(i)+base_address);
					printf("rep local : %08x\n", repd+get_remoteness(i)+base_address);
					continue;
				}
				char *name = symbol_attr_find(p,attr);
				int img;
				symbol_t t = symbol_find(name,&img);
				int rmtns = get_remoteness(img);
				rep_entry_write(p,i,j,t.attribute[0]+base_address+rmtns);
				printf("name : %s\nfinal address : %08x\n\n",t.name,t.attribute[0]+base_address+rmtns);
			}
		}
		puts("");
		printf("img %d \"%s\"\nfinal address : %08x\nsize : %08x\n\n",i,imagename[i],base_address+remoteness,imagesize[i]-(p->repaddr + p->repno * 4));
		remoteness += imagesize[i]-(p->repaddr + p->repno * 4);
	}
	
	FILE *fp = fopen(argv[1],"wb");
	
	for(i = 0; i < readed_n; i++) {
		coffhead_t *p = (coffhead_t *)image[i];
		fwrite(image[i]+(p->repaddr + p->repno * 4),1,imagesize[i]-(p->repaddr + p->repno * 4),fp);
	}
	
	fclose(fp);
	
	return 0;
}
