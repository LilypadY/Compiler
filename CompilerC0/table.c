#include<stdio.h>
#include<string.h>
#include"err.h"
#include"table.h"
#include"lex.h"
struct symbol_table tab[TMAX];
int tp = 0;
struct block_table btab[BMAX];
int bp = 0;

int global_var = 0,global_array = 0;
void global_var_arr();

void table_insert(char name[],int type,int kind,int addr,int array_len){
    if(tp==TMAX){
        error(SMT_ERR,SYMTABOVERFLOW);
        return;
    }
    if(search_cur_blk(name)>=0){
        error(SMT_ERR,RENAMEERR);
        return;
    }
    tab[tp].kind = kind;
    if(type==VOIDSY)
        tab[tp].type = TP_NO;
    else{
        tab[tp].type = type-INTSY+TP_INT;
    }
    strcpy(tab[tp].name,name);
    switch(kind){
        case CONST:
            tab[tp].addr = addr;//constant value
            btab[bp].consts ++;
            break;
        case VAR:
            tab[tp].tmp_var=0;
            btab[bp].vars ++;
            break;
        case PARAM:
            tab[tp].addr = btab[bp].params++;
            break;
        case FUNC:
            if(!bp) global_var_arr();
            bp ++;
            btab[bp].head = tp;
            tab[tp].refb = bp;
            btab[bp].ap = 1;
            btab[bp].params = 0;
            btab[bp].vars = 0;
            break;
        case ARRAY:
            btab[bp].vars ++;
            tab[tp].array_len = array_len;
            tab[tp].refb = btab[bp].ap;
            btab[bp].arrays[btab[bp].ap].len = array_len;
            btab[bp].arrays[btab[bp].ap].idx = btab[bp].ap;
            btab[bp].ap++;
            break;
    }
    tp ++;
}
int lookup(char name[]){
    int j;
    int base = btab[bp].head;
    int cur = search_cur_blk(name);
    if(cur>=0){
        return cur;
    }
    //search other func names and global vars and constants
    for(j = base;j >= 0;j --){
        if(tab[j].kind==PARAM) continue;
        if(!strcmp(name,tab[j].name)){
            return j;
        }
    }
    return -1;
}
int search_cur_blk(char name[]){
    int j;
    int base = btab[bp].head;
    int cl = btab[bp].consts;
    int pl = btab[bp].params;
    //search the current block
    for(j = btab[bp].vars+cl+pl+base;j >base; j--){
        if(!strcmp(name,tab[j].name)){
            return j;
        }
    }
    return -1;
}
void popblk(){
    tp = btab[bp].head+btab[bp].params+1;
    //bp++;
}
void init_tab(){//global block
    btab[0].head = -1;
    btab[0].params = 0;
    btab[0].vars = 0;
    btab[0].ap = 1;
}
void global_var_arr(){
    int i,cont = 1;
    for(i = 0;tab[i].kind != FUNC;i ++){
        if(tab[i].kind==VAR){
            tab[i].tmp_var = -cont++;
        }
        else if(tab[i].kind==ARRAY){
            tab[i].refb *= -1;
        }
    }
    for(i = 1;i < btab[0].ap; i++){
        btab[0].arrays[i].idx *= -1;
    }
}
void output_table(){
    fprintf(fw,"-------------\n");
    fprintf(fw,"tab\n\n");
    int i;
    for(i = 0;i < tp;i ++){
        fprintf(fw,"%s\tkind=%d\ttype=%d\taddr=%d\tarray_len=%d\tref=%d\n",tab[i].name,tab[i].kind,tab[i].type,tab[i].addr,tab[i].array_len,tab[i].refb);
    }
    fprintf(fw,"btab\n\n");
    fprintf(fw,"bp=%d\n",bp);
    for(i = 0;i <= bp;i ++){
        fprintf(fw,"head=%d\tparams=%d\tconsts=%d\tvars=%d\n",btab[i].head,btab[i].params,btab[i].consts,btab[i].vars);
    }
    fprintf(fw,"-------------\n");
}
