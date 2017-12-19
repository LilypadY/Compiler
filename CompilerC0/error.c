#include<stdio.h>
#include"err.h"
#include"lex.h"
char *err_type[]={"Lexical error","Syntax error","Semantic Error","Memory Error"};
int errs = 0;
int skipblk_flag = 0,skipline_flag = 0;
void error(int type,char* info){
    //info <0
    printf("ERROR:\t%s in line %d.\t%s\n",err_type[type],lineno,info);
    errs ++;
}
int skip(int target){
    while(symbol&&symbol!=SMCLSY&&symbol!=RBRCSY&&symbol!=target) symbol = nextsym();
    if(symbol!=target){
        symbol = nextsym();
        return 0;
    }
    return 1;
}
void skippart(int target){
    while(symbol&&symbol!=RBRCSY&&symbol!=target) symbol = nextsym();
}
void skipblk(){
    while(symbol) {
        if(symbol==RBRCSY){
            symbol = nextsym();
            if(symbol==INTSY||symbol==CHARSY||symbol==VOIDSY){
                break;
            }
        }
        else
            symbol = nextsym();

    }
    skipblk_flag = 1;
}
void skipline(){
    while(symbol&&symbol!=SMCLSY&&symbol!=RBRCSY) symbol = nextsym();
    symbol = nextsym();
    skipline_flag = 1;
}
