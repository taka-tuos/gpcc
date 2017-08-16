#include "as.h"
#include <stdlib.h>

#define INVALID		-0x7fffffff

static char opcode_name[][32] = {
	"0nop",
	"2mov",
	"2add",
	"2sub",
	"2imul",
	"2idiv",
	"2and",
	"2or",
	"1not",
	"2xor",
	"1int",
	"1call",
	"0ret",
	"1jmp",
	"2cmp",
	"2sar",
	"2sal",
	"1je",
	"1jae",
	"1jbe",
	"1ja",
	"1jb",
	"1jz",
	"1jnz",
	"1jne",
	"2in",
	"2out",
	"1push",
	"1pop",
	"0cli",
	"0sti",
	"2fmov",
	"2fadd",
	"2fsub",
	"2fmul",
	"2fdiv",
	"2farjmp",
	"2farcall",
	"0farret",
	"1lsp",
	"1ssp",
	"2hmxs",
	"1hmxc",
	"2hmxi",
	"1htxs",
	"1htxl",
	"2htxp",
	"2umul",
	"2udiv",
	"\x7f"
};

static int size_type[] = {
	1,
	1,
	4,
	4,
	1,
	4,
	1,
	5,
	2,
	5,
	2
};

char str[1024];
args_t args_m[2];
label_t label[512];
int last_label = 0;
symbol_t symbol[4096];
int replace_addr[65536];
int last_replace = 0;
int last_symbol = 0;
int now_address = 0;
int base_address = 0;
int db_size = 0;
int scan_label = 0;
int scan_symbol = 0;
int now_line = 0;
char db_data[16777216];

char *get_op(char *s);
args_t get_argtype2(char *s);
args_t get_argtype(char *s,int num);
char *get_argstr(char *s,int num);
char get_bytecode(char *s);
int get_labeladdr(char *s);
int get_symboladdr(char *s);
binary_t get_binary(char *s);
void errmsg(char *sz);

int main(int argc,char *argv[])
{
	FILE *fp0;
	FILE *fp1;
	binary_t bin;
	char code[1024];
	int mode;
	
	memset(symbol,0,sizeof(symbol));
	memset(label,0,sizeof(label));

	if(argc < 3) {
		puts("usage>as <source-file> <binary-file> [-flat|-coff]");
		exit(1);
	}

	now_line = -255;
	
	if(strcmp(argv[3],"-flat") == 0) mode = 0;
	else if(strcmp(argv[3],"-coff") == 0) mode = 1;
	else errmsg("unknown format option");
	
	fp0 = fopen(argv[1],"rt");
	fp1 = fopen(argv[2],"wb");
	
	now_line = 0;
	scan_label = 0;
	scan_symbol = 0;

	int ptr = 0;
	
	do {
		now_line++;
		fgets(code,1024,fp0);
		char *p = code;
		for(;*p==' '||*p==0x09;p++);
		if(*p == ';') continue;
		if(*p == 0x0d || *p == 0x0a) continue;
		if(*p == 0x00 || *p == EOF) continue;
		get_binary(code);
	} while (!feof(fp0));

	fseek(fp0,0,SEEK_SET);
	now_address = 0;
	now_line = 0;
	
	scan_label = 1;
	scan_symbol = 1;
	
	do {
		now_line++;
		fgets(code,1024,fp0);
		char *p = code;
		for(;*p==' '||*p==0x09;p++);
		if(*p == ';') continue;
		if(*p == 0x0d || *p == 0x0a) continue;
		if(*p == 0x00 || *p == EOF) continue;
		get_binary(code);
	} while (!feof(fp0));
	
	fseek(fp0,0,SEEK_SET);
	now_address = 0;
	now_line = 0;
	
	scan_label = 1;
	scan_symbol = 0;
	
	char *image = (char *)malloc(16777216);

	do {
		now_line++;
		fgets(code,1024,fp0);
		char *p = code;
		for(;*p==' '||*p==0x09;p++);
		if(*p == ';') continue;
		if(*p == 0x0d || *p == 0x0a) continue;
		if(*p == 0x00 || *p == EOF) continue;
		bin = get_binary(code);
		if(bin.bytes!=0) memcpy(image+ptr,bin.bin,bin.bytes);//fwrite(bin.bin,sizeof(char),bin.bytes,fp1);
		ptr += bin.bytes;
	} while (!feof(fp0));
	
	if(mode) {
		int repaddr = 4 * 4 + last_symbol * sizeof(symbol_t);
		fprintf(fp1,"GP32");
		fwrite(&last_symbol,sizeof(int),1,fp1);
		fwrite(&repaddr,sizeof(int),1,fp1);
		fwrite(&last_replace,sizeof(int),1,fp1);
		fwrite(symbol,sizeof(symbol_t),last_symbol,fp1);
		fwrite(replace_addr,sizeof(int),last_replace,fp1);
	}
	fwrite(image,sizeof(char),ptr,fp1);

	fclose(fp0);
	fclose(fp1);

	return 0;
}

void replacepoint(int offset)
{
	replace_addr[last_replace] = now_address + offset;
	last_replace++;
}

int hash(char *sz)
{
	int i;
	for(i = 0;*sz;sz++) i += *sz;
	return i;
}

void errmsg(char *sz)
{
	printf("%s at line %d\n",sz, now_line);
	exit(hash(sz));
}

char *skipspace(char *p)
{
	for (; *p == ' '; p++) { }	/* スペースを読み飛ばす */
	return p;
}

int getnum(char **pp, int priority)
{
	char *p = *pp;
	int i = INVALID, j;
	p = skipspace(p);

	/* 単項演算子 */
	if (*p == '+') {
		p = skipspace(p + 1);
		i = getnum(&p, 0);
	} else if (*p == '-') {
		p = skipspace(p + 1);
		i = getnum(&p, 0);
		if (i != INVALID) {
			i = - i;
		}
	} else if (*p == '~') {
		p = skipspace(p + 1);
		i = getnum(&p, 0);
		if (i != INVALID) {
			i = ~i;
		}
	} else if (*p == '(') {	/* かっこ */
		p = skipspace(p + 1);
		i = getnum(&p, 9);
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
			j = getnum(&p, 2);
			if (j != INVALID) {
				i += j;
			} else {
				i = INVALID;
			}
		} else if (*p == '-' && priority > 2) {
			p = skipspace(p + 1);
			j = getnum(&p, 2);
			if (j != INVALID) {
				i -= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '*' && priority > 1) {
			p = skipspace(p + 1);
			j = getnum(&p, 1);
			if (j != INVALID) {
				i *= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '/' && priority > 1) {
			p = skipspace(p + 1);
			j = getnum(&p, 1);
			if (j != INVALID && j != 0) {
				i /= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '%' && priority > 1) {
			p = skipspace(p + 1);
			j = getnum(&p, 1);
			if (j != INVALID && j != 0) {
				i %= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '<' && p[1] == '<' && priority > 3) {
			p = skipspace(p + 2);
			j = getnum(&p, 3);
			if (j != INVALID && j != 0) {
				i <<= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '>' && p[1] == '>' && priority > 3) {
			p = skipspace(p + 2);
			j = getnum(&p, 3);
			if (j != INVALID && j != 0) {
				i >>= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '&' && priority > 4) {
			p = skipspace(p + 1);
			j = getnum(&p, 4);
			if (j != INVALID) {
				i &= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '^' && priority > 5) {
			p = skipspace(p + 1);
			j = getnum(&p, 5);
			if (j != INVALID) {
				i ^= j;
			} else {
				i = INVALID;
			}
		} else if (*p == '|' && priority > 6) {
			p = skipspace(p + 1);
			j = getnum(&p, 6);
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

char *get_op(char *s)
{
	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++) {
		str[i]=*p;
		if(*p == 0) errmsg("unable find to \'(\'"); //)
	}

	str[i]=0;

	return str;
}

char *get_argstr(char *s,int num)
{
	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++) {
		if(*p == 0) errmsg("unable find to \'(\'"); //)
	}

	p++;

	for(i=0;i<num;i++) {
		for(;*p!=',';p++); //(
		if(*p == 0) errmsg("unable find to \',\' or \')\'");
		p++;
	}

	for(i=0;*p!=','&&*p!=')';p++,i++) {
		if(*p == 0) errmsg("unable find to \',\' or \')\'");
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

char *get_argstr2(char *s)
{
	int i;
	char *p = s;

	for(;*p==' '||*p==0x09;p++);
	for(i = 0;*p!='(';p++,i++) {
		if(*p == 0) errmsg("unable find to \'(\'"); //)
	}
	p++;

	for(i=0;*p!=')';p++,i++) {
		if(*p == 0) errmsg("unable find to \',\' or \')\'");
		str[i]=*p;
	}

	str[i]=0;

	return str;
}

int g_ofs;

args_t get_argtype2(char *s)
{
	int i;
	args_t args;
	char strs[128];
	char rs[16], as[16];
	long m, a;
	int rsn;
	char *n;

	char *d = s;
	char t;
	
	switch(*d) {
	case '%':
		d++;
		t = *d;
		if(t == 'r') args.type = ARGS_IR;
		else if(t == 's') args.type = ARGS_SR;
		else errmsg("unknown register prefix at memory acsess");
		d++;
		for(i=0;*d!=0;d++,i++) strs[i]=*d;
		strs[i]=0;
		args.num = atoi(strs);
		if(args.num >= 36) {
			errmsg("overrun register number at memory acsess");
		}
		break;
	case '@':
		d++;
		if(*d != '[') errmsg("after \'@\' must be \'[\' at offset memory acsess");
		d++;
		if(*d != '%') errmsg("after \'@[\' must be \'%\' at offset memory acsess");
		for(i = 0; *d != ':'; d++, i++) rs[i] = *d;
		rs[i] = 0;
		d++;
		for(i = 0; *d != ']'; d++, i++) as[i] = *d;
		as[i] = 0;
		if((as[0] >= '0' && as[0] <= '9') || as[0] == '-') args.type = ARGS_RAE;
		else if(as[0] == '.') args.type = ARGS_RAE;
		else if(as[0] == '%') args.type = ARGS_RAR;
		else errmsg("unknown identifier at offset memory acsess");
		n = as;
		if(as[0] == '%') {
			n++;
			if(as[0] != 'r') errmsg("base register must be general at offset memory acsess");
			n++;
			for(i=0;*n!=0;n++,i++) strs[i]=*n;
			strs[i]=0;
			a = atoi(strs);
		} else if(as[0] == '.') {
			for(i=0;*n!=0;n++,i++) strs[i]=*n;
			strs[i]=0;
			a = get_labeladdr(strs);
			if(a < 0) errmsg("unknown label identifier at offset memory access");
			if(scan_label && !scan_symbol) replacepoint(3+g_ofs);
		} else {
			a = getnum(&n,9);
		}
		rsn = atoi(rs+2);
		if(rsn >= 36) {
			errmsg("overrun register number at offset memory acsess");
		}
		args.num = ((UINT64)(rsn)) | ((UINT64)a << 8);
		break;
	case '$':
		args.type = ARGS_E;
		//args.num = strtol(d+1,(char **)&strs,0);
		d++;
		args.num = getnum(&d, 9);
		break;
	case '.':
		args.type = ARGS_E;
		for(i=0;d[i]!=0 && d[i]!=',' && d[i]!=')';i++) strs[i]=d[i];
		strs[i]=0;
		if(scan_label && !scan_symbol) {
			int q = get_labeladdr(strs);
			if(q < 0) {
				m = get_symboladdr(strs);
				if(m >= 0) {
					args.num = symbol[m].attribute[0];
				} else {
					errmsg("unknown label identifier at memory access");
				}
			} else {
				args.num = q;
			}
			if(scan_label && !scan_symbol) printf("READ(MEM) %s : %08llx\n",strs,args.num);
			if(scan_label && !scan_symbol) replacepoint(2+g_ofs);
		}
		break;
	default:
		errmsg("unknown argments type at memory access");
	}

	return args;
}

args_t get_argtype(char *s,int num)
{
	args_t args;
	args_t arg2;
	int i;
	char *d = get_argstr(s,num);
	char strs[128];
	char bufn[1024];
	char rs[16], as[16];
	char t;
	long m, a;
	int rsn;
	char *n;
	gp3200_float f;
	
	strcpy(bufn,s);
	int ofs = num ? size_type[(int)args_m[0].type] : 0;
	g_ofs = ofs;
	
	switch(*d) {
	case '%':
		d++;
		t = *d;
		if(t == 'r') args.type = ARGS_IR;
		else if(t == 'f') args.type = ARGS_FR;
		else if(t == 's') args.type = ARGS_SR;
		else errmsg("unknown register prefix");
		d++;
		for(i=0;d[i]!=0;i++) strs[i]=d[i];
		strs[i]=0;
		d = strs;
		args.num = atoi(d);
		if(args.num >= 36) {
			errmsg("overrun register number");
		}
		args.size = 4;
		break;
	case 'b':
		d++;
		//if(*d != '@') d++;
		arg2 = get_argtype2(d);
		if(arg2.type == ARGS_E) args.type = ARGS_ME;
		if(arg2.type == ARGS_IR) args.type = ARGS_MR;
		if(arg2.type == ARGS_RAE) args.type = ARGS_MRAE;
		if(arg2.type == ARGS_RAR) args.type = ARGS_MRAR;
		args.num = arg2.num;
		args.size = 1;
		break;
	case 'w':
		d++;
		//if(*d != '@') d++;
		arg2 = get_argtype2(d);
		if(arg2.type == ARGS_E) args.type = ARGS_ME;
		if(arg2.type == ARGS_IR) args.type = ARGS_MR;
		if(arg2.type == ARGS_RAE) args.type = ARGS_MRAE;
		if(arg2.type == ARGS_RAR) args.type = ARGS_MRAR;
		args.num = arg2.num;
		args.size = 2;
		break;
	case 'd':
		d++;
		//if(*d != '@') d++;
		arg2 = get_argtype2(d);
		if(arg2.type == ARGS_E) args.type = ARGS_ME;
		if(arg2.type == ARGS_IR) args.type = ARGS_MR;
		if(arg2.type == ARGS_RAE) args.type = ARGS_MRAE;
		if(arg2.type == ARGS_RAR) args.type = ARGS_MRAR;
		args.num = arg2.num;
		args.size = 4;
		break;
	case '$':
		args.type = ARGS_E;
		d++;
		//args.num = strtol(d,(char **)&strs,0);
		args.num = getnum(&d, 9);
		args.size = 4;
		break;
	case '@':
		d++;
		if(*d != '[') errmsg("after \'@\' must be \'[\'");
		d++;
		if(*d != '%') errmsg("after \'@[\' must be \'%\'");
		for(i = 0; *d != ':'; d++, i++) rs[i] = *d;
		rs[i] = 0;
		d++;
		for(i = 0; *d != ']'; d++, i++) as[i] = *d;
		as[i] = 0;
		if((as[0] >= '0' && as[0] <= '9') || as[0] == '-') args.type = ARGS_RAE;
		else if(as[0] == '.') args.type = ARGS_RAE;
		else if(as[0] == '%') args.type = ARGS_RAR;
		else errmsg("unknown identifier at offset number");
		n = as;
		if(as[0] == '%') {
			n++;
			n++;
			for(i=0;*n!=0;n++,i++) strs[i]=*n;
			strs[i]=0;
			a = atoi(strs);
		} else if(as[0] == '.') {
			for(i=0;*n!=0;n++,i++) strs[i]=*n;
			strs[i]=0;
			a = get_labeladdr(strs);
			if(a < 0) errmsg("unknown label identifier at offset number");
			if(scan_label && !scan_symbol) replacepoint(3+ofs);
		} else a = getnum(&n,9);
		rsn = atoi(rs+2);
		if(rsn >= 36) {
			errmsg("overrun register number at offset number");
		}
		args.num = ((UINT64)(rsn)) | ((UINT64)a << 8);
		args.size = 4;
		break;
	case '#':
		args.type = ARGS_FE;
		f = gp3200_f32_float2f32(atof(d+1));
		args.num = *((UINT32 *) &f);
		args.size = 4;
		break;
	case '.':
		args.type = ARGS_E;
		for(i=0;d[i]!=0 && d[i]!=',' && d[i]!=')';i++) strs[i]=d[i];
		strs[i]=0;
		if(scan_label && !scan_symbol) {
			int q = get_labeladdr(strs);
			if(q < 0) {
				m = get_symboladdr(strs);
				if(m >= 0) {
					args.num = symbol[m].attribute[0];
				} else {
					errmsg("unknown label identifier");
				}
			} else {
				args.num = q;
			}
			if(scan_label && !scan_symbol) printf("READ(IMM) %s : %08llx\n",strs,args.num);
			if(scan_label && !scan_symbol) replacepoint(2+ofs);
		}
		args.size = 4;
		break;
	default:
		errmsg("unknown argments type");
	}

	return args;
}

char get_bytecode(char *s)
{
	int i;
	int j;
	args_t arg;
	char *d = get_op(s);
	char buf0[1024];
	char strs[128];
	char *e;

	if(strncmp(d,"org",3) == 0 || strncmp(d,"ORG",3) == 0) {
		arg = get_argtype(s,0);
		if(arg.type != ARGS_E) errmsg("org address must be const");
		now_address = arg.num;
		base_address = arg.num;
		return 0xff;
	}

	if(strncmp(d,"equ",3) == 0 || strncmp(d,"EQU",3) == 0) {
		arg = get_argtype(s,1);
		if(arg.type != ARGS_E) errmsg("equ data must be const");
		e = get_argstr(s,0);
		for(i=0;i<256 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
			label[last_label].name[i]=e[i];
		}
		label[last_label].address = arg.num;
		last_label++;
		return 0x6f;
	}

	if(strncmp(d,"db",2) == 0 || strncmp(d,"DB",2) == 0) {
		arg = get_argtype(s,0);
		if(arg.type != ARGS_E) errmsg("data must be const");
		db_size = 1;
		db_data[0] = arg.num;
		return 0x7f;
	}
	
	if(strncmp(d,"dw",2) == 0 || strncmp(d,"DW",2) == 0) {
		arg = get_argtype(s,0);
		if(arg.type != ARGS_E) errmsg("data must be const");
		db_size = 2;
		*((short *)db_data) = arg.num;
		return 0x7f;
	}
	
	if(strncmp(d,"dd",2) == 0 || strncmp(d,"DD",2) == 0) {
		arg = get_argtype(s,0);
		if(arg.type != ARGS_E) errmsg("data must be const");
		db_size = 4;
		*((int *)db_data) = arg.num;
		return 0x7f;
	}
	
	if(strncmp(d,"fill",2) == 0 || strncmp(d,"FILL",2) == 0) {
		arg = get_argtype(s,0);
		if(arg.type != ARGS_E) errmsg("data must be const");
		db_size = arg.num;
		memset(db_data,0,db_size);
		return 0x7f;
	}
	
	if(strncmp(d,"global",2) == 0 || strncmp(d,"GLOBAL",2) == 0) {
		//arg = get_argtype(s,0);
		int addr;
		e = get_argstr(s,0);
		strcpy(strs,e);
		if(scan_symbol) {
			addr = get_labeladdr(strs);
			if(addr < 0) {
				if(get_symboladdr(strs) >= 0) errmsg("global label already defined");
				for(i=0;i<256 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
					symbol[last_symbol].name[i]=e[i];
				}
				symbol[last_symbol].attribute[0] = last_symbol + 0x40000000;
				printf("EXTERN %s : %08x\n",symbol[last_symbol].name,symbol[last_symbol].attribute[0]);
				last_symbol++;
			} else {
				if(get_symboladdr(strs) >= 0) errmsg("global label already defined");
				arg = get_argtype(s,0);
				if(arg.type != ARGS_E) errmsg("global label must be const");
				for(i=0;i<256 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
					symbol[last_symbol].name[i]=e[i];
				}
				symbol[last_symbol].attribute[0] = addr - base_address;
				printf("GLOBAL %s : %08x\n",symbol[last_symbol].name,symbol[last_symbol].attribute[0]);
				last_symbol++;
			}
		}
		return 0xff;
	}
	
	if(strncmp(d,"align",2) == 0 || strncmp(d,"ALIGN",2) == 0) {
		int i;
		arg = get_argtype(s,0);
		if(arg.type != ARGS_E) errmsg("fill length must be const");
		for(i = 0; ((now_address + i) % arg.num) != 0; i++);
		db_size = i;
		db_data[0] = 0;
		return 0x7f;
	}

	if(strncmp(d,"text",4) == 0 || strncmp(d,"TEXT",4) == 0) {
		db_size = strlen(get_argstr2(s));
		db_size++;
		memcpy(db_data,get_argstr2(s),db_size);
		sprintf(buf0,db_data);
		strcpy(db_data,buf0);
		db_size = strlen(db_data) + 1;
		return 0x7f;
	}

	for(i=0;;i++) {
		if(*d>='A'&& *d<='Z') {
			for(j=0;j<strlen(d);j++) {
				d[j]+=0x20;
			}
		}
		if(opcode_name[i][0] == 0x7f) errmsg("unknown instruction");
		if(strcmp(d,opcode_name[i]+1)==0) break;
	}

	return i;
}

int get_labeladdr(char *s)
{
	int i;
	char *d = s;

	for(i=0;i<512;i++) {
		if(label[i].name[0] != 0 && strcmp(d,label[i].name)==0) return label[i].address;
	}

	return -1;
}

int get_symboladdr(char *s)
{
	int i;
	char *d = s;

	for(i=0;i<4096;i++) {
		if(symbol[i].name[0] != 0 && strcmp(d,symbol[i].name)==0) return i;
	}

	return -1;
}

binary_t get_binary(char *s)
{
	int i;
	binary_t bin;
	char type[2];
	char *p = s;

	memset(bin.bin,0,256);
	memset(args_m,0,sizeof(args_m));

	if(!scan_label && !scan_symbol) {
		if(*s=='.') {
			for(i=0;i<256 && s[i]!=0x0d && s[i]!=0x0a && s[i]!=0;i++) {
				label[last_label].name[i]=s[i];
			}
			label[last_label].address = now_address;
			printf("DEFINE %s : %08x\n",label[last_label].name,now_address);
			bin.bytes=0;
			last_label++;
			return bin;
		}
	} else {
		if(*s=='.') {
			bin.bytes=0;
			return bin;
		}
	}

	for(;*p==' '||*p==0x09;p++);
	
	if(*p==';'||*p==0x0d||*p==0x0a) {
		bin.bytes=0;
		return bin;
	}

	unsigned char opcode = get_bytecode(s);
	char argsnum = opcode_name[opcode][0]-'0';

	if(opcode==0xff) {
		bin.bytes=0;
		now_address+=bin.bytes;
		return bin;
	}

	if(opcode==0x6f) {
		bin.bytes=0;
		now_address+=bin.bytes;
		return bin;
	}
	
	if(opcode==0x7f) {
		memcpy(bin.bin,db_data,db_size);
		bin.bytes = db_size;
		db_size=0;
		now_address+=bin.bytes;
		return bin;
	}

	if(argsnum==0) {
		bin.bin[0] = opcode << 2;
		bin.bytes=1;
		now_address+=bin.bytes;
		return bin;
	}

	for(i=0;i<argsnum;i++) {
		args_m[i] = get_argtype(s,i);
		type[i]=args_m[i].type;
	}

	bin.bin[0] = (opcode << 2) | ((args_m[0].size - 1) & 0x03);
	if(args_m[0].type != ARGS_MR || args_m[0].type != ARGS_ME || args_m[0].type != ARGS_MRAE || args_m[0].type != ARGS_MRAR) bin.bin[0] = (opcode << 2) | ((args_m[1].size - 1) & 0x03);

	if(argsnum==1) {
		bin.bin[1]=type[0];
		bin.bytes=1+1+size_type[(int)type[0]];
		*((unsigned int *)&bin.bin[2])=args_m[0].num;
	} else if(argsnum==2) {
		bin.bin[1]=((char)type[1] << 4)|(char)type[0];
		bin.bytes=1+1+size_type[(int)type[0]]+size_type[(int)type[1]];
		*((UINT64 *)&bin.bin[2])=args_m[0].num;
		*((UINT64 *)&bin.bin[size_type[(int)type[0]]+2])=args_m[1].num;
	}
	
	now_address+=bin.bytes;
	return bin;
}
