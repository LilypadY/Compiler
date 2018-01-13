#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED
#include"compile.h"
/*kind */
#define KIND_ST (0)
#define CONST (KIND_ST+0)
#define VAR (KIND_ST+1)
#define PARAM (KIND_ST+2)
#define FUNC (KIND_ST+3)
#define ARRAY (KIND_ST+4)

#define TP_INT 0
#define TP_CHAR 1
#define TP_NO 2

#define AMAX (20)
#define TMAX (1000)
#define BMAX (100)


struct symbol_table{
    char name[MAX_ID];
    int kind;
    int type;
    int addr;
    //value of constant,offset by vars in terms of block head,offset by params in terms of block head,index of code of function
    int array_len;
    int refb;
    int tmp_var;//tmp var name(no.)
};
struct arr_info{
    int len;
    int idx;//idx in arrays in btab
};
struct block_table{
    int head;//head declare in tab
    int params;//counts of parameters
    int consts;
    int vars;//counts of var record
    int vd;//last var in tmp_var in data stack,start from 1
    struct arr_info arrays[AMAX];
    int ap;
    int code_st,code_en;
};

struct symbol_table tab[TMAX];
extern int tp ;
struct block_table btab[BMAX];
extern int bp;

extern int tmp_var_cont;

extern int global_var,global_array;

void table_insert(char name[],int type,int kind,int addr,int array_len);
int lookup(char name[]);
void init_tab();
void output_table();
void popblk();
int search_cur_blk(char name[]);

#endif // TABLE_H_INCLUDED
