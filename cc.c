#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BINPATH "."

#define OPTION(s) strncmp(argv[i],s,strlen(s)) == 0

int hash(char *sz)
{
	int i;
	for(i = 0;*sz;sz++) i += *sz;
	return i;
}

int main(int argc, char *argv[])
{
	char arg_cc1[4096];
	char arg_cpp[4096];
	char arg_as[4096];
	char arg_ast[512];
	char sz[512];
	char option_format[64] = "-flat";
	int i;
	int next = 0;
	int ret;
	int ptr = 0;
	
	if(argc < 3) {
		puts("Error : too few argments");
		printf("usage : %s [option] <sourcename> <outputname>\n",argv[0]);
		return 3;
	}
	
	strcpy(arg_cc1,BINPATH "/cc1/cc1 ");
	strcpy(arg_cpp,BINPATH "/cpp/cpp ");
	strcpy(arg_as,BINPATH "/as/as ");
	
	for(i = 1; i < argc; i++) {
		if(next > 0) {
			strcat(arg_cpp,argv[i]);
			strcat(arg_cpp," ");
			next = 0;
			continue;
		}
		if(
			OPTION("-I") ||
			OPTION("-J")
		) {
			strcat(arg_cpp,argv[i]);
			strcat(arg_cpp," ");
			next = 1;
		} else if(
			OPTION("-C") ||
			OPTION("-s") ||
			OPTION("-l") ||
			OPTION("-CC") ||
			OPTION("-a") ||
			OPTION("-na") ||
			OPTION("-V") ||
			OPTION("-u") ||
			OPTION("-X") ||
			OPTION("-c90") ||
			OPTION("-w") ||
			OPTION("-zl") ||
			OPTION("-M") ||
			OPTION("-D") ||
			OPTION("-U") ||
			OPTION("-A") ||
			OPTION("-B") ||
			OPTION("-Y") ||
			OPTION("-Z") ||
			OPTION("-d") ||
			OPTION("-e") ||
			OPTION("-v") ||
			OPTION("-h")
		) {
			strcat(arg_cpp,argv[i]);
			strcat(arg_cpp," ");
		} else if(
			OPTION("-flat") ||
			OPTION("-coff")
		) {
			strcpy(option_format,argv[i]);
		} else {
			if(ptr < 2) {
				if(ptr) {
					strcpy(arg_ast,argv[i]);
				}
				if(!ptr) {
					strcat(arg_cpp,argv[i]);
					strcat(arg_cpp," ");
				}
			} else {
				puts("Error : too many filenames");
				return 1;
			}
			ptr++;
		}
	}
	
	if(ptr < 2) {
		puts("Error : unable to get output filename");
		return 2;
	}
	
	srand(hash(arg_ast));
	
	int tmp1 = ((rand()+rand()) << 16) | (rand()+rand());
	int tmp2 = tmp1 + 1;
	
	sprintf(sz,"%08x.tmp %s %s",tmp2,arg_ast,option_format);
	strcat(arg_as,sz);
	
	sprintf(sz,"-o %08x.tmp",tmp1);
	strcat(arg_cpp,sz);
	
	sprintf(sz,"%08x.tmp %08x.tmp",tmp1,tmp2);
	strcat(arg_cc1,sz);
	
	sprintf(sz,"rm %08x.tmp %08x.tmp",tmp1,tmp2);
	
	puts(arg_cpp);
	puts(arg_cc1);
	puts(arg_as);
	
	system(arg_cpp);
	system(arg_cc1);
	system(arg_as);
	
	//system(sz);
	
	return 0;
}
