#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include"compile.h"
#include"optimize.h"
#define MAXPATH 1000


int main()
{
    debug = 1;
    char filename[MAXPATH]="test.txt";
    printf("please input full path of file\n");
    if(!debug) scanf("%s",filename);
    FILE* fp = fopen(filename,"rb");
    fw = fopen("code4.txt","w");
    fw_opt = fopen("opt_code4.txt","w");
    fw_opt_mips = fopen("opt_code.txt","w");
    if(fp==NULL){
		fprintf(fw,"FILE OPEN ERROR:\tFile not exist\n");
		return 0;
	}
	fseek(fp,0,SEEK_END);
	f_len = ftell(fp);
	rewind(fp);

	buf = (char *)malloc(sizeof(char)*(f_len+1));
	if(buf==NULL){
		fprintf(fw,"Memory Error:\tFile too large\n");
		return 0;
	}
	int res = fread(buf,sizeof(char),f_len,fp);
	if(res!=f_len){
		fprintf(fw,"File Read Error\n");
		return 0;
	}
	fclose(fp);
	buf[f_len] = '\0';
	pt = 0;
	program();
	free(buf);
	fclose(fw);
	mips_gene();
    dismLocExp();
    //fclose(fw);
    fclose(fw_opt);
    fclose(fw_opt_mips);
    return 0;
}
