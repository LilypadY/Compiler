#ifndef COMPILE_H_INCLUDED
#define COMPILE_H_INCLUDED

/*file content*/
char* buf;
long f_len;
int lineno;
int pt ;
FILE* fw;
FILE* fcode;
FILE* fw_opt_mips;
FILE* fw_opt;

#define MAX_STR_LEN (1000)
#define MAX_ID (30)

int intsy_value;
char charsy_value;
char strsy_value[MAX_STR_LEN];
char idsy_value[MAX_ID];

int symbol;

int nextsym();
void program();
void mips_gene();

int debug;


#endif // COMPILE_H_INCLUDED
