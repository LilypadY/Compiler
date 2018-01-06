#include<stdio.h>
#include"err.h"
#include"lex.h"
char *err_type[]={"Lexical error","Syntax error","Semantic Error","Memory Error"};
int errs = 0;
int skipblk_flag = 0,skipline_flag = 0;
static int brace = 0;
void error(int type,char* info){
    //info <0
    //if(!errs)
    if(symbol==RBRCSY){
        if(brace>0){
            brace--;
            return;
        }
    }
    if(type==STX_ERR)
        printf("ERROR:\t%s in line %d.word%d\t%s\n",err_type[type],lineno,symbol,info);
    else
        printf("ERROR:\t%s in line %d.\t%s\n",err_type[type],lineno,info);
    errs ++;
}
int skip(int target){
    while(symbol&&symbol!=SMCLSY&&symbol!=RBRCSY&&symbol!=target){
        if(symbol==LBRCSY) brace++;
        if(symbol==RBRCSY) brace--;
        symbol = nextsym();
    }
    if(symbol!=target){
        symbol = nextsym();
        return 0;
    }
    return 1;
}
void skip2(int target){
    while(symbol&&symbol!=SMCLSY&&symbol!=RBRCSY&&symbol!=target) {
            if(symbol==LBRCSY) brace++;
            if(symbol==RBRCSY) brace--;
            symbol = nextsym();
    }
}
void skippart(int target){
    while(symbol&&symbol!=RBRCSY&&symbol!=target){
            if(symbol==LBRCSY) brace++;
            if(symbol==RBRCSY) brace--;
            symbol = nextsym();
    }
}
void skipblk(){
    while(symbol) {
        if(symbol==RBRCSY){
            symbol = nextsym();
            if(symbol==INTSY||symbol==CHARSY||symbol==VOIDSY){
                break;
            }
        }
        else{
            symbol = nextsym();
        }
    }
    skipblk_flag = 1;
}
void skipline(){
    while(symbol&&symbol!=SMCLSY&&symbol!=RBRCSY) {
            if(symbol==LBRCSY) brace++;
            if(symbol==RBRCSY) brace--;
            symbol = nextsym();
    }
    symbol = nextsym();
    skipline_flag = 1;
}
