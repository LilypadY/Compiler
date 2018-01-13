#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"lex.h"
#include"err.h"
#include"table.h"
#include"memory.h"
#define CALLMAX 200

int ischar(int c){
    if(c=='+'||c=='-'||c=='*'||c=='/'||c=='_'||(c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')) return 1;
    return 0;
}

int* expression();
void statement();
void statements();
void assign_statement();
void while_statement();
void scanf_statement();
void printf_statement();
void return_statement();
int* factor();
void func_call();

int ret_state_type = 0;
int exp_type = 0;//0 for int,1 for char

int integer(){
    int flag = 1,value = 0;
    if(symbol==PLUSSY||symbol==MINUSSY){
        if(symbol==MINUSSY) flag = -1;
        symbol = nextsym();
        if(symbol==ZEROSY){
            error(STX_ERR,SZERO);
        }
    }
    if(symbol!=INTVALSY&&symbol!=ZEROSY){
        error(STX_ERR,UINTMS);
        skipline();
        return 0;
    }
    if(symbol==INTVALSY) value = flag*intsy_value;
    symbol = nextsym();
    if(debug){
        fprintf(fw,"this is an integer\n");
    }
    return value;
}

void const_decl(){
    //int err_recs = errs;
    int type,value;
    char name[MAX_ID];
    while(symbol==CONSTSY){
        symbol = nextsym();
        type = symbol;
        if(type!=CHARSY&&type!=INTSY){
            error(STX_ERR,TYPEERR);//invalid const type
            skip(IDSY);
            type=INTSY;
            if(symbol!=IDSY) {
                    skipline();
                    skipline_flag = 0;
                    continue;
            }
        }
        else symbol = nextsym();
        if(symbol!=IDSY){
            error(STX_ERR,IDSYERR);//invalid identifier
            skip(IDSY);
            if(symbol!=IDSY) {
                    skipline();
                    skipline_flag = 0;
                    continue;
            }
        }
        strcpy(name,idsy_value);
        symbol = nextsym();
        if(symbol!=ASNSY){
            error(STX_ERR,ASNERR);
            skip(0);
            continue;
        }
        symbol = nextsym();
        if(type==INTSY){
            value = integer();
        }
        else if(type==CHARSY){
            if(symbol!=CHARVALSY){
                error(STX_ERR,CONSTASNMISMATCH);
                skip(0);
                continue;
            }
            symbol = nextsym();
            value = (int)charsy_value;
        }
        else {
            error(STX_ERR,CONSTUNINIT);//uninitialized constant
            symbol = nextsym();
        }
        table_insert(name,type,CONST,value,0);
        while(symbol==COMMASY){
            symbol = nextsym();
            if(symbol!=IDSY){
                error(STX_ERR,IDSYERR);//invalid identifier
                skip(COMMASY);
                continue;
            }
            strcpy(name,idsy_value);
            symbol = nextsym();
            if(symbol!=ASNSY){
                error(STX_ERR,ASNERR);//uninitialized constant
                skip(COMMASY);
                continue;
            }
            symbol = nextsym();
            if(type==INTSY){
                value = integer();
            }
            else if(type==CHARSY){
                if(symbol!=CHARVALSY){
                    error(STX_ERR,CONSTASNMISMATCH);
                    skip(COMMASY);
                    continue;
                }
                value = charsy_value;
                symbol = nextsym();
            }
            else {
                error(STX_ERR,TYPEERR);
                symbol = nextsym();
                continue;
            }
            table_insert(name,type,CONST,value,0);
        }
        if(symbol!=SMCLSY){
            error(STX_ERR,SMCLERR);
            skip(SMCLSY);
        }
        else{
           symbol = nextsym();
        }
    }
    if(debug){
        fprintf(fw,"This is a constant declaration\n");
    }
}
void var_oneline(int type,char headname[]){
    int kind = VAR,array_len = 0;
    char name[MAX_ID];
    if(symbol==LBRKSY){
        kind = ARRAY;
        symbol = nextsym();
        if(symbol!=INTVALSY){
            error(STX_ERR,ARRLENERR);//array index error
            skipline();
            skipline_flag = 0;
            return;
        }
        else array_len = intsy_value;
        symbol = nextsym();
        if(symbol!=RBRKSY){
            error(STX_ERR,RBKERR);//] not found
            skipline();
            skipline_flag = 0;
            return;
        }
        symbol = nextsym();
    }
    else{
        kind = VAR;
    }
    table_insert(headname,type,kind,0,array_len);

    while(symbol==COMMASY){
        symbol = nextsym();
        if(symbol!=IDSY) {
            error(STX_ERR,IDSYERR);//identifier not found
            skip(COMMASY);
            continue;
        }
        else strcpy(name,idsy_value);
        symbol = nextsym();
        if(symbol==LBRKSY){
            symbol = nextsym();
            if(symbol!=INTVALSY){
                error(STX_ERR,ARRLENERR);//array index error
                skip(COMMASY);
                continue;
            }
            else array_len = intsy_value;
            kind = ARRAY;
            symbol = nextsym();
            if(symbol!=RBRKSY){
                error(STX_ERR,RBKERR);//] not found
                skip2(COMMASY);
                continue;
            }
            symbol = nextsym();
        }
        else{
            kind = VAR;
        }
        table_insert(name,type,kind,0,array_len);
    }
}
int var_decl(int headtype,char headname[]){//only store 1st name in line
    //int err_recs = errs;
    int type;
    char name[MAX_ID];
    if(symbol==COMMASY||symbol==SMCLSY||symbol==LBRKSY)
        var_oneline(headtype,headname);
    while(symbol){
        if(symbol!=SMCLSY){
            error(STX_ERR,SMCLERR);
            skip(SMCLSY);
            if(symbol!=SMCLSY)
                return 0;
        }
        symbol = nextsym();
        if(symbol==IFSY||symbol==DOSY||symbol==LBRCSY||symbol==SWITCHSY||symbol==IDSY||symbol==SCANFSY||symbol==PRINTFSY||symbol==RETURNSY||symbol==SMCLSY||symbol==RBRCSY){
            if(debug){
                fprintf(fw,"this is a var declaration\n");
            }
            return symbol;
        }
        if(symbol==VOIDSY){
            type = VOIDSY;
            return VOIDSY;
        }

        if(symbol!=INTSY&&symbol!=CHARSY){
            error(STX_ERR,TYPESYERR);//type symbol not found
            skip(IDSY);
            type = INTSY;
            if(symbol!=IDSY){
                skip(SMCLSY);
                continue;
            }
        }
        else {
                type = symbol;
                symbol = nextsym();
        }
        if(symbol!=IDSY){
            error(STX_ERR,IDSYERR);//name not found
            skip(IDSY);
            if(symbol!=IDSY){
                skip(SMCLSY);
                continue;
            }
        }
        if(symbol==IDSY) strcpy(name,idsy_value);
        symbol = nextsym();
        if(symbol==LPARSY){
            strcpy(headname,name);//return name of function into input char*(name array in main)
            symbol = nextsym();
            if(debug){
                fprintf(fw,"this is a var declaration\n");
            }
            return type;
        }
        if(symbol==COMMASY||symbol==SMCLSY||symbol==LBRKSY)
            var_oneline(type,name);
        else{
            error(STX_ERR,INVALID);
            skipline();
            skipline_flag = 0;
            return 0;
        }
    }
    return 0;
}
void param_list(){
    int type;
    //int err_recs = errs;
    if(symbol!=RPARSY){
        if(symbol!=INTSY&&symbol!=CHARSY) {
                error(STX_ERR,TYPEERR);
                type=INTSY;
                if(!skip(IDSY)){
                    skipline_flag = 1;
                    return;
                }
        }
        else {
                type = symbol;
                symbol = nextsym();
        }
        if(symbol!=IDSY){
            error(STX_ERR,IDSYERR);
            if(!skip(RPARSY)){
                skipline_flag = 1;
                return;
            }
        }
        else {
            table_insert(idsy_value,type,PARAM,0,0);
            symbol = nextsym();
        }
        while(symbol==COMMASY){
            symbol = nextsym();
            if(symbol!=INTSY&&symbol!=CHARSY){
                error(STX_ERR,TYPEERR);
                type=INTSY;
                if(!skip(IDSY)){
                    skipline_flag = 1;
                    return;
                }
            }
            else {
                type = symbol;
                symbol = nextsym();
            }
            if(symbol!=IDSY){
                error(STX_ERR,IDSYERR);
                if(!skip(RPARSY)){
                    skipline_flag = 1;
                    return;
                }
                else{
                    break;
                }
            }
            else table_insert(idsy_value,type,PARAM,0,0);
            symbol = nextsym();
        }
    }
    if(symbol!=RPARSY) {
            error(STX_ERR,RPERR);
            skip(RPARSY);
    }
    else
        symbol = nextsym();
    if(debug) fprintf(fw,"This is a param_list\n");
}
int* factor(){
    //int err_recs = errs;
    int* value = NULL;
    if(symbol==CHARVALSY){
        value = (int*)malloc(sizeof(int));
        *value = (int)charsy_value;
        symbol = nextsym();
        return value;
    }
    else if(symbol==PLUSSY||symbol==MINUSSY||symbol==INTVALSY||symbol==ZEROSY){
        value = (int*)malloc(sizeof(int));
        *value = integer();
        if(skipline_flag){
            return NULL;
        }
        if(exp_type==1) exp_type = 0;
    }
    else if(symbol==LPARSY){
        symbol = nextsym();
        int cur_type = exp_type;
        value = expression();
        if(cur_type==0||exp_type==0) exp_type = 0;
        if(skipline_flag){
            return NULL;
        }
        if(symbol!=RPARSY){
            error(STX_ERR,RPERR);
            skip(RPARSY);
        }
        else
            symbol = nextsym();
    }

    else if(symbol==IDSY){
        int tabidx = lookup(idsy_value);
        int tv;
        if(tabidx<0){
            error(SMT_ERR,UNDEFINE);
            skipblk();
            return NULL;
        }
        if(tab[tabidx].kind==CONST){
            value = (int*)malloc(sizeof(int));
            *value = tab[tabidx].addr;
            symbol = nextsym();
            if(exp_type==1&&tab[tabidx].type!=TP_CHAR) exp_type = 0;
            return value;
        }
        //if(debug) if(tab[tabidx].kind==FUNC) printf("func type = %d exptype=%d ",tab[tabidx].type==TP_CHAR,exp_type);
        if(exp_type==1&&tab[tabidx].type!=TP_CHAR) exp_type = 0;
        if(tab[tabidx].kind==PARAM){
            emit2(MOVE,tab[tabidx].addr,GLV, F_PARAM,F_VAR);
            symbol = nextsym();
            return NULL;
        }
        symbol = nextsym();
        if(symbol==LBRKSY){
            if(tab[tabidx].kind!=ARRAY){
                error(SMT_ERR,NOTARR);
                skipblk();
                return NULL;
            }
            symbol = nextsym();
            int cur_exptype = exp_type;
            int* idxvalue = expression();
            if(skipblk_flag||skipline_flag){
                return NULL;
            }
            if(exp_type==1){
                error(SMT_ERR,TYPEMISMATCH);
                skipline();
                skipline_flag = 0;
                return NULL;
            }
            exp_type = cur_exptype;
            if(symbol!=RBRKSY){
                error(STX_ERR,RBKERR);
                skip(RBRKSY);
            }
            int arridx = tab[tabidx].refb;
            if(idxvalue!=NULL){
                if((*idxvalue)<0|| (*idxvalue)>=tab[tabidx].array_len){
                    error(SMT_ERR,ARRIDXERR);
                    skipblk();
                    return NULL;
                }
                else{
                    emit3(GETA,arridx,*idxvalue,GLV, F_ARR,F_VAL);
                }
            }
            else{
                emit3(GETA,arridx,GLV,GLV, F_ARR,F_VAR);
            }
            symbol = nextsym();
        }
        else if(symbol==LPARSY){
            if(tab[tabidx].kind!=FUNC){
                error(SMT_ERR,NOTFUNC);
                skipblk();
                return NULL;
            }
            if(tab[tabidx].type==TP_NO){
                error(SMT_ERR,FUNCCALL);
                skipblk();
                return NULL;
            }
            else{
                int cur_type = exp_type;
                func_call(tabidx);
                exp_type = cur_type;
            }
        }
        else if(symbol==MULSY||symbol==DIVSY||symbol==PLUSSY||symbol==MINUSSY||symbol==RPARSY||symbol==RBRKSY||symbol==RBRCSY||symbol==COLONSY||ISCMP(symbol)||symbol==COMMASY||symbol==SMCLSY){
            if(tab[tabidx].tmp_var) tv = tab[tabidx].tmp_var;
            else {
                tv = apply_t();
                tab[tabidx].tmp_var = tv;
            }
            emit2(MOVE,tv,GLV, F_VAR,F_VAR);
        }
        else{
            error(STX_ERR, FACTORERR);
            skipline();
            return NULL;
        }
    }
    else{
        error(STX_ERR, FACTORERR);
        skipline();
        return NULL;
    }

    if(debug) fprintf(fw,"this is a factor\n");
    return value;
}
int* item(){
    //int err_recs = errs;
    int *value = NULL,*factor_value;
    int op;
    //order
    int res = -1;
    factor_value = factor();
    if(skipblk_flag||skipline_flag){
        return NULL;
    }
    if(factor_value!=NULL){
        value = factor_value;
    }
    else{
        res  = apply_t();
        emit2(MOVE,GLV,res, F_VAR,F_VAR);
    }
    while(symbol==MULSY||symbol==DIVSY){
        if(exp_type==1) exp_type = 0;
        op = symbol;
        symbol = nextsym();
        factor_value = factor();
        if(skipblk_flag||skipline_flag){
            return NULL;
        }
        if(value!=NULL&&factor_value!=NULL){
            if(op==MULSY) (*value) *= (*factor_value);
            else (*value) /= (*factor_value);
            free(factor_value);
        }
        else if(value!=NULL&&factor_value==NULL){
            res  = apply_t();
            if(op==MULSY) emit3(MUL,*value,GLV,res, F_VAL,F_VAR);
            else emit3(DIV,*value,GLV,res, F_VAL,F_VAR);
            free(value);
            value=NULL;
        }
        else if(value==NULL&&factor_value!=NULL){
             if(op==MULSY) emit3(MUL,res,*factor_value,res, F_VAR,F_VAL);
             else emit3(DIV,res,*factor_value,res, F_VAR,F_VAL);
             free(factor_value);
        }
        else{
            if(op==MULSY) emit3(MUL,res,GLV,res, F_VAR,F_VAR);
             else emit3(DIV,res,GLV,res, F_VAR,F_VAR);
        }
    }
    if(res!=-1){
        emit2(MOVE,res,GLV, F_VAR,F_VAR);
    }
    if(debug) fprintf(fw,"This is an item \n");
    return value;
}
int* expression(){
    //int err_recs = errs;
    int *value = NULL,*item_value;
    int res = -1;
    int headsign = 1,op;
    exp_type = 1;
    if(symbol==PLUSSY||symbol==MINUSSY){
        if(symbol==MINUSSY) headsign = -1;
        symbol = nextsym();
        exp_type = 0;
    }
    item_value = item();
    if(skipblk_flag||skipline_flag){
        return NULL;
    }
    if(item_value!=NULL){
        value = item_value;
        (*value) *= headsign;
    }
    else{
        res = apply_t();
        emit2(MOVE,GLV,res, F_VAR,F_VAR);
        if(headsign<0){
            emit3(SUB,0,res,res, F_VAL,F_VAR);
        }
    }
    while(symbol==PLUSSY||symbol==MINUSSY){
        if(exp_type==1) exp_type = 0;
        op = symbol;
        symbol = nextsym();
        item_value = item();
        if(skipblk_flag||skipline_flag){
            return NULL;
        }
        if(value!=NULL&&item_value!=NULL){
            if(op==PLUSSY) (*value) += (*item_value);
            else (*value) -= (*item_value);
            free(item_value);
        }
        else if(value!=NULL&&item_value==NULL){
            res  = apply_t();
            if(op==PLUSSY) emit3(ADD,GLV,*value,res,F_VAR, F_VAL);
            else emit3(SUB,*value,GLV,res, F_VAL,F_VAR);
            free(value);
            value=NULL;
        }
        else if(value==NULL&&item_value!=NULL){
             if(op==PLUSSY) emit3(ADD,res,*item_value,res, F_VAR,F_VAL);
             else emit3(SUB,res,*item_value,res, F_VAR,F_VAL);
             free(item_value);
        }
        else{
            if(op==PLUSSY) emit3(ADD,res,GLV,res, F_VAR,F_VAR);
             else emit3(SUB,res,GLV,res, F_VAR,F_VAR);
        }
    }
    if(res!=-1){
        emit2(MOVE,res,GLV, F_VAR,F_VAR);
    }
    if(debug) fprintf(fw,"This is an expression \n");
    return value;
}

void func_call(int tabidx){
    //int err_recs = errs;
    //symbol==(
    struct order4 pushlist[CALLMAX];
    int base = btab[tab[tabidx].refb].head;
    int ps = btab[tab[tabidx].refb].params;
    int k = 0;
    int *value;
    do{
        symbol = nextsym();
        if(!k&&symbol==RPARSY){
            break;
        }
        value = expression();
        if(skipblk_flag||skipline_flag){
            return;
        }
        k ++;
        if(tab[base+k].type==TP_CHAR && exp_type==0){
            error(SMT_ERR,TYPEMISMATCH);
            skipline();
            return;
        }
        else{
            if(value!=NULL){
                pushlist[k-1].x = *value;
                pushlist[k-1].flag_x = F_VAL;
                free(value);
            }
            else{
                int pr = apply_t();
                emit2(MOVE,GLV,pr, F_VAR,F_VAR);
                pushlist[k-1].x = pr;
                pushlist[k-1].flag_x = F_VAR;
            }
        }
        if(k>ps||(k==ps&&symbol==COMMASY)){
            error(SMT_ERR,TOOMANYPARAM);
            skipline();
            return;
        }
    }while(symbol==COMMASY);
    if(k<ps){
        error(SMT_ERR,TOOLITPARAM);
    }
    if(symbol!=RPARSY){
        error(STX_ERR,RPERR);
        skip(RPARSY);
    }
    symbol = nextsym();
    for(k = 0;k < ps;k ++){
        emit2(PUSH,pushlist[k].x, k, pushlist[k].flag_x,F_VAL);
    }
    emit1(CALL,tab[tabidx].refb,F_FUNC);
    if(debug)  fprintf(fw,"This is a function_call statement\n");
}

void condition(int pos_branch){
    //int err_recs = errs;
    int *value = expression();
    int e1,e2,op,flag1,flag2;
    if(skipblk_flag||skipline_flag){
        return ;
    }
    flag1 = (value==NULL)?F_VAR:F_VAL;
    if(symbol==RPARSY) {
        e1 = (value==NULL)?GLV:(*value);
        if(pos_branch) emit3(BNE,e1,0,-1, flag1,F_VAL);
        else emit3(BEQ,e1,0,-1, flag1,F_VAL);
        return;
    }
    if(value!=NULL){
        e1 = *value;
    }
    else{
        e1 = apply_t();
        emit2(MOVE,GLV,e1, F_VAR,F_VAR);
    }
    if(!ISCMP(symbol)){
        error(STX_ERR,CMPSPT);
        skip(RPARSY);
        return;
    }
    else{
        op = symbol;
    }
    symbol=nextsym();
    value = expression();
    if(skipblk_flag||skipline_flag){
        return;
    }
    if(value!=NULL){
        e2 = *value;
        flag2 = F_VAL;
    }
    else{
        e2 = GLV;
        flag2 = F_VAR;
    }

    switch(op){
        case LSTSY:
            if(pos_branch) emit3(BLT,e1,e2,-1, flag1,flag2);
            else emit3(BGE,e1,e2,-1, flag1,flag2);
            break;
        case GTHSY:
            if(pos_branch) emit3(BGT,e1,e2,-1, flag1,flag2);
            else emit3(BLE,e1,e2,-1, flag1,flag2);
            break;
        case LEQSY:
            if(pos_branch) emit3(BLE,e1,e2,-1, flag1,flag2);
            else emit3(BGT,e1,e2,-1, flag1,flag2);
            break;
        case GEQSY:
            if(pos_branch) emit3(BGE,e1,e2,-1, flag1,flag2);
            else emit3(BLT,e1,e2,-1, flag1,flag2);
            break;
        case NEQSY:
            if(pos_branch) emit3(BNE,e1,e2,-1, flag1,flag2);
            else emit3(BEQ,e1,e2,-1, flag1,flag2);
            break;
        case EQUSY:
            if(pos_branch) emit3(BEQ,e1,e2,-1, flag1,flag2);
            else emit3(BNE,e1,e2,-1, flag1,flag2);
            break;
    }
    if(debug) fprintf(fw,"This is a condition statement\n");
}
void if_statement(){
    //int err_recs = errs;
    if(symbol!=IFSY){
        return;
    }
    symbol=nextsym();
    if(symbol!=LPARSY){
        error(STX_ERR,LPERR);
        skipline();
        skipline_flag = 0;
        return;
    }
    symbol=nextsym();
    condition(0);
    int cd_cp = c4p-1;
    if(skipblk_flag){
        return;
    }
    if(skipline_flag){
        skipline_flag = 0;
    }
    else if(symbol!=RPARSY){
        error(STX_ERR,RPERR);
        skip(RPARSY);
        symbol=nextsym();
    }
    else
        symbol=nextsym();
    if(symbol==IFSY||symbol==DOSY||symbol==LBRCSY||symbol==SWITCHSY||symbol==IDSY||symbol==SCANFSY||symbol==PRINTFSY||symbol==RETURNSY||symbol==SMCLSY||symbol==RBRCSY)
        statement();
    if(skipblk_flag||skipline_flag){
        return;
    }
    int label = apply_label();
    fill_Border_to(label,cd_cp);//order following if block
    if(debug) fprintf(fw,"This is an if statement\n");
}
void switch_statement(){
    //int err_recs = errs;
    symbol=nextsym();
    if(symbol!=LPARSY){
        error(STX_ERR,LPERR);
        skip(LPARSY);
    }
    symbol=nextsym();
    int *value = expression();
    if(skipblk_flag||skipline_flag){
        return ;
    }
    int etype = exp_type;
    int sw_val,sw_flag;
    int label,lastc4p = -1;
    int cs_cont = 0,i;
    int cslist[100],cs_exit_lbl[100];
    if(skipblk_flag||skipline_flag){
        skipline_flag = 0;
        return ;
    }
    if(value==NULL){
        sw_val = apply_t();
        sw_flag = F_VAR;
        emit2(MOVE,GLV,sw_val, F_VAR,F_VAR);
    }
    else{
        sw_val = *value;
        sw_flag = F_VAL;
    }

    if(symbol!=RPARSY){
        error(STX_ERR,RPERR);
        skip(LBRCSY);
        if(symbol!=LBRCSY){
            symbol=nextsym();
            return;
        }
    }
    symbol = nextsym();

    if(symbol!=LBRCSY){
        error(STX_ERR,LBRCERR);
        skip(CASESY);
    }
    else
        symbol=nextsym();
    while(symbol&&symbol!=RBRCSY){
        if(symbol!=CASESY){
            error(STX_ERR,CASEERR);
            skippart(CASESY);
            if(symbol!=CASESY){
                return;
            }
        }
        if(cs_cont){
            label = apply_label();
            fill_Border_to(label,lastc4p);//label of next bne
        }
        symbol=nextsym();
        if(symbol==CHARVALSY){
                if(etype==0){
                    error(SMT_ERR,TYPEERR);
                    skippart(CASESY);
                    continue;
                }
                for(i = 0;i < cs_cont; i ++){
                    if(cslist[i]==charsy_value){
                        error(SMT_ERR,REPCASEERR);
                        skippart(CASESY);
                        break;
                    }
                }
                if(i<cs_cont){
                    continue;
                }
                lastc4p = emit3(BNE,sw_val,charsy_value,0 ,sw_flag,F_VAL);
                cslist[cs_cont] = charsy_value;
                symbol=nextsym();
        }
        else if(symbol==PLUSSY||symbol==MINUSSY||symbol==INTVALSY||symbol==ZEROSY){
            if(etype!=0){
                error(SMT_ERR,TYPEERR);
                skippart(CASESY);
                continue;
            }
            int value = integer();
            if(skipline_flag){
                skippart(CASESY);
                continue;
            }
            for(i = 0;i < cs_cont; i ++){
                    if(cslist[i]==value){
                        error(SMT_ERR,REPCASEERR);
                        skippart(CASESY);
                        break;
                    }
                }
                if(i<cs_cont){
                    continue;
                }
            lastc4p = emit3(BNE,sw_val,value,0 ,sw_flag,F_VAL);
            cslist[cs_cont ] = value;
        }
        else{
            error(STX_ERR,CSCONSTERR);
            skippart(CASESY);
            continue;
        }

        if(symbol!=COLONSY){
            error(STX_ERR,COLONERR);
        }
        symbol=nextsym();
        statement();
        if(skipblk_flag){
            return ;
        }
        cs_exit_lbl[cs_cont++]=emit0(J);
    }
    c4p --;//no branch statement need for last case statement
    label = apply_label();
    fill_Border_to(label,lastc4p);
    for(i = 0;i < cs_cont-1;i ++){
        fill_Border_to(label,cs_exit_lbl[i]);
    }
    if(symbol!=RBRCSY){
        error(STX_ERR,RBRCERR);
        skip(0);
    }
    symbol=nextsym();
    if(debug) fprintf(fw,"This is a switch statement\n");
}
void scanf_statement(){
    //int err_recs = errs;
    int tabidx;

    symbol=nextsym();
    if(symbol!=LPARSY){
        error(STX_ERR,LPERR);
    }
    do{
        symbol=nextsym();
        if(symbol!=IDSY){
            error(STX_ERR,IDSYERR);
            if(!skip(IDSY)){
                return;
            }
        }

        tabidx = lookup(idsy_value);
        if(tabidx<0){
            error(SMT_ERR,UNDEFINE);
            skipline();
            skipline_flag = 0;
            return;
        }
        if(tab[tabidx].kind!=VAR&&tab[tabidx].kind!=PARAM){
            error(SMT_ERR,TYPEMISMATCH);
            symbol=nextsym();
            continue;
        }
        int tv;
        if(tab[tabidx].tmp_var) tv = tab[tabidx].tmp_var;
        else {
            tv = apply_t();
            tab[tabidx].tmp_var = tv;
        }
        if(tab[tabidx].type==TP_CHAR){
            emit0(SYS_READCH);
            emit2(MOVE,GLV,tv, F_VAR,F_VAR);
        }
        else{
            emit0(SYS_READINT);
            emit2(MOVE,GLV,tv, F_VAR,F_VAR);
        }
        symbol=nextsym();
    }while(symbol==COMMASY);
    if(symbol!=RPARSY){
        error(STX_ERR,RPERR);
    }
    symbol=nextsym();
    if(symbol!=SMCLSY){
        error(STX_ERR,SMCLERR);
    }
    symbol=nextsym();
    if(debug) fprintf(fw,"This is a scanf statement\n");
}

void printf_statement(){
    //int err_recs = errs;
    int strno = -1,strflag = 0;
    symbol=nextsym();
    if(symbol!=LPARSY){
        error(STX_ERR,LPERR);
        skip(0);
        return;
    }
    symbol=nextsym();
    if(symbol==STRVALSY){
        strno = strlp;
        strflag = 1;
        strcpy(stringlist[strlp++],strsy_value);
        //print string
        symbol=nextsym();
        if(symbol!=RPARSY){
            if(symbol!=COMMASY){
                error(STX_ERR,COMMAERR);
                skipline();
                skipline_flag = 0;
                return;
            }
            else{
                strflag = 2;
                symbol=nextsym();
            }

        }
    }
    if(strflag==1){
        emit1(SYS_PRINTSTR,strno,F_STR);
        //emit1(SYS_PRINTSTR,-1,F_STR);
    }
    else{
        int *value = expression();
        if(skipblk_flag||skipline_flag){
            skipline_flag = 0;
            return;
        }
        if(strflag==2){
            emit1(SYS_PRINTSTR,strno,F_STR);
        }
        if(value!=NULL){
            if(exp_type==1){
                emit1(SYS_PRINTCH,*value,F_VAL);
            }
            else
                emit1(SYS_PRINTINT,*value,F_VAL);
        }
        else{
            if(exp_type==1)
                emit1(SYS_PRINTCH,GLV,F_VAR);
            else
                emit1(SYS_PRINTINT,GLV,F_VAR);
        }
        //emit1(SYS_PRINTSTR,-1,F_STR);
    }
    if(symbol!=RPARSY){
        error(STX_ERR,RPERR);
    }
    symbol=nextsym();
    if(symbol!=SMCLSY){
        error(STX_ERR,SMCLERR);
        skipline();
        skipline_flag = 0;
        return;
    }
    else symbol=nextsym();
    if(debug) fprintf(fw,"This is a printf statement\n");
}
void return_statement(){
    //int err_recs = errs;
    symbol=nextsym();
    int blktabidx = btab[bp].head;
    int func_type = tab[blktabidx].type;
    if(symbol==LPARSY){
        symbol=nextsym();
        int *value = expression();
        if(skipblk_flag||skipline_flag){
            skipline_flag = 0;
            return;
        }
        if(exp_type==0&&func_type==TP_CHAR){
            error(SMT_ERR,TYPEMISMATCH);
            symbol = nextsym();
        }
        if(symbol!=RPARSY){
            error(STX_ERR,RPERR);
        }
        if(value!=NULL){
            emit1(RET,*value,F_VAL);
        }
        else{
            emit1(RET,GLV,F_VAR);
        }
        symbol=nextsym();//;
    }
    else{
        if(func_type!=TP_NO){
            error(SMT_ERR,INVALIDRET);
            skip2(SMCLSY);
        }
        emit0(RET0);
    }
    if(symbol!=SMCLSY){
        error(STX_ERR,SMCLERR);
    }
    symbol=nextsym();
    ret_state_type = 1;
    if(debug) fprintf(fw,"This is a return statement\n");
}
void assign_statement(char name[]){
    //int err_recs = errs;
    int tabidx = lookup(name);
    int idxv,arr_fv = -1,valv,valfv;
    if(tabidx<0){
        error(SMT_ERR,UNDEFINE);
        skipline();
        skipline_flag = 0;
        return;
    }
    if(symbol==LBRKSY){
        if(tab[tabidx].kind!=ARRAY){
            error(SMT_ERR,NOTARR);
            skip(0);
            return;
        }
        else{
            symbol=nextsym();
            int *idxvalue = expression();
            if(skipblk_flag||skipline_flag){
                skipline_flag = 0;
                return;
            }
            if(exp_type==1){
                error(SMT_ERR,ARRIDXERR);
                skipline();
                skipline_flag = 0;
                return;
            }
            if(idxvalue!=NULL){
                if(*idxvalue>=tab[tabidx].array_len){
                    error(SMT_ERR,ARRIDXERR);
                    skipline();
                    skipline_flag = 0;
                    return;
                }
                idxv = *idxvalue;
                arr_fv = F_VAL;
            }
            else{
                idxv = apply_t();
                emit2(MOVE,GLV,idxv, F_VAR,F_VAR);
                arr_fv = F_VAR;
            }
            if(symbol!=RBRKSY){
                error(STX_ERR,RBKERR);
            }
            symbol=nextsym();
            if(symbol!=ASNSY){
                error(STX_ERR,ASNERR);
            }
        }
    }
    else{
        if(tab[tabidx].kind!=VAR&&tab[tabidx].kind!=PARAM){
            error(SMT_ERR,NOTVAR);
            skipline();
            skipline_flag = 0;
            return;
        }
    }
    symbol=nextsym();
    int *value = expression();
    if(skipblk_flag||skipline_flag){
        skipline_flag = 0;
        return;
    }
    if(value!=NULL){
        valv = *value;
        valfv = F_VAL;
        if(tab[tabidx].type==TP_CHAR){
            if(!ischar(*value)){
                error(SMT_ERR,TYPEMISMATCH);
                skipline();
                skipline_flag = 0;
                return;
            }
        }
    }
    else{
        /*if(tab[tabidx].type==TP_CHAR&&exp_type!=1){
            error(SMT_ERR,TYPEMISMATCH);
            skipline();
            skipline_flag = 0;
            return;
        }*/
        valv = GLV;
        valfv = F_VAR;
    }
    if(arr_fv>=0){
        int tarr=tab[tabidx].refb;
        emit3(ASNA,idxv,valv,tarr,  arr_fv,valfv);//z[x] = y
    }
    else{
        int tv;
        if(tab[tabidx].kind==VAR){
            if(tab[tabidx].tmp_var){
                tv = tab[tabidx].tmp_var;
            }
            else{
                tv = apply_t();
                tab[tabidx].tmp_var = tv;
            }
            emit2(MOVE,valv,tv, valfv,F_VAR);
        }
        else if(tab[tabidx].kind==PARAM){
            emit2(MOVE,valv,tab[tabidx].addr, valfv,F_PARAM);
        }
    }
    if(symbol!=SMCLSY){
        error(STX_ERR,SMCLERR);
        skip(SMCLSY);
    }
    if(symbol==SMCLSY)
        symbol=nextsym();
    if(debug) fprintf(fw,"This is an assign statement\n");
}
void while_statement(){
    //int err_recs = errs;
    symbol=nextsym();
    int label = apply_label();
    statement();
    if(skipblk_flag){
        return;
    }
    if(skipline_flag){
        skipline_flag = 0;
    }
    if(symbol!=WHILESY){
        error(STX_ERR,WHILEERR);
        skipblk();
    }
    symbol=nextsym();
    if(symbol!=LPARSY) error(STX_ERR,LPERR);//int err_recs
    symbol=nextsym();
    condition(1);
    if(skipblk_flag||skipline_flag ){
        skipline_flag = 0;
        return;
    }
    if(symbol!=RPARSY){
        error(STX_ERR,LPERR);
    }
    fill_Border(label);
    symbol=nextsym();
    if(debug) fprintf(fw,"This is a while statement\n");
}

void statement(){
    //int err_recs = errs;
    char name[MAX_ID];
    switch(symbol){
            case IFSY:if_statement();break;
            case SWITCHSY:switch_statement();break;
            case DOSY:while_statement();break;
            case SCANFSY:scanf_statement();break;
            case PRINTFSY:printf_statement();break;
            case RETURNSY:return_statement();break;
            case SMCLSY:symbol=nextsym();break;
            case LBRCSY:
                symbol=nextsym();
                statements();
                if(skipblk_flag){
                    return;
                }
                if(symbol!=RBRCSY)
                    error(STX_ERR,RBRCERR);
                symbol=nextsym();
                break;
            case IDSY:{
                strcpy(name,idsy_value);
                symbol = nextsym();
                if(symbol==LPARSY){
                        int tabidx = lookup(name);
                        if(tabidx<0){
                            error(SMT_ERR,UNDEFINE);
                            skipblk();
                            return;
                        }
                        else if(tab[tabidx].kind!=FUNC){
                            error(SMT_ERR,NOTFUNC);
                            skipblk();
                            return;
                        }
                        else {
                                func_call(tabidx);
                                if(skipblk_flag||skipline_flag){
                                    return ;
                                }
                        }
                        if(symbol!=SMCLSY){
                            error(STX_ERR,SMCLERR);
                            skip(SMCLSY);
                        }
                        else symbol=nextsym();
                        break;
                }
                else if(symbol==ASNSY||symbol==LBRKSY) {
                        assign_statement(name);break;}
                else{ error(STX_ERR,INVALID);skipline();skipline_flag=0;break;}
            }
            default:error(STX_ERR,INVALID);skipline();skipline_flag=0;break;
        }
        if(debug) fprintf(fw,"This is a statement\n");
}
void statements(){
    //int err_recs = errs;
    while(symbol&&symbol!=RBRCSY){
        statement();
        if(skipblk_flag){
            return;
        }
    }
    if(debug) fprintf(fw,"This is statements\n");
    //symbol = nextsym();
}
void compound_statement(){
    //int err_recs = errs;
    char name[MAX_ID];
    int type;

    if(symbol==CONSTSY){
        const_decl();
    }
    if(symbol==INTSY||symbol==CHARSY){
        type = symbol;
        symbol = nextsym();
        if(symbol!=IDSY){
            error(STX_ERR,IDSYERR);
            skipline();
            skipline_flag = 0;
        }
        else{
            strcpy(name,idsy_value);
            symbol = nextsym();
            int var_nxt = var_decl(type,name);
            if(var_nxt==INTSY||var_nxt==CHARSY||var_nxt==VOIDSY){
                error(STX_ERR,FCTINSTM);
                skipblk();
                skipblk_flag = 0;
                return;
            }
        }

    }
    statements();
    if(skipblk_flag){
        return;
    }
    if(debug) fprintf(fw,"This is a compound statement\n");

}

void func(int func_type,char name[]){
    //int err_recs = errs;
    table_insert(name,func_type,FUNC,0,0);
    tmp_var_cont = 1;
    ret_state_type = 0;
    param_list();
    if(skipline_flag){
        skipblk_flag = 0;
    }
    else if(symbol!=LBRCSY) error(STX_ERR,LBRCERR);
    symbol = nextsym();
    tab[btab[bp].head].addr = c4p;
    btab[bp].code_st = c4p;
    compound_statement();
    if(skipblk_flag){
        skipblk_flag = 0;
        return;
    }
    if(func_type==INTSY||func_type==CHARSY){
        //check return statement type
        if(!ret_state_type){
            error(SMT_ERR,NORET);
        }
    }
    if(code4[c4p-1].f!=RET) emit0(RET0);
    if(symbol!=RBRCSY) error(STX_ERR,RBRCERR);
    symbol = nextsym();
    btab[bp].vd = tmp_var_cont;
    btab[bp].code_en = c4p;
    if(debug) output_table();
    popblk();
    if(debug) fprintf(fw,"This is a function\n");
}
void func_main(){
    //int err_recs = errs;
    table_insert("main",VOIDSY,FUNC,0,0);
    tmp_var_cont = 1;
    if(symbol!=LPARSY) error(STX_ERR,LPERR);
    symbol = nextsym();
    if(symbol!=RPARSY) error(STX_ERR,RPERR);
    symbol = nextsym();
    if(symbol!=LBRCSY) error(STX_ERR,LBRCERR);
    symbol = nextsym();
    tab[btab[bp].head].addr = c4p;
    btab[bp].code_st = c4p;
    compound_statement();
    if(skipblk_flag){
        skipblk_flag = 0;
        return;
    }
    emit0(RET0);
    if(symbol!=RBRCSY) error(STX_ERR,RBRCERR);
    btab[bp].vd = tmp_var_cont;
    btab[bp].code_en = c4p;
    if(debug) output_table();

    popblk();
    if(debug) fprintf(fw,"This is a func_main\n");
}

void program(){
    //int err_recs = errs;
    init_tab();
    int type=0;//type of function or variable
    char name[MAX_ID];

    symbol = nextsym();
    if(symbol==CONSTSY){
        const_decl();
    }
    if(symbol==INTSY||symbol==CHARSY){
        type = symbol;
        symbol = nextsym();
        if(symbol!=IDSY){
            error(STX_ERR,IDSYERR);
            //name of var or function expected
        }
        else{
            strcpy(name,idsy_value);
            symbol = nextsym();
            if(symbol==LPARSY){
                symbol = nextsym();
                func(type,name);//paramlist starting
            }
            else if(symbol==LBRKSY||symbol==COMMASY||symbol==SMCLSY){
                int var_nxt = var_decl(type,name);
                if(var_nxt==INTSY||var_nxt==CHARSY){
                    func(var_nxt,name);
                }
                else if(var_nxt==0){
                    error(STX_ERR,MSMAIN);
                }
                else if(var_nxt!=VOIDSY){
                    error(STX_ERR,GLBSTM);//statement out of function
                }
            }
            else {
                error(STX_ERR,INVALID);
            }
        }
    }
    while(1){
        if(symbol==0){
            error(STX_ERR,MSMAIN);
            break;
        }
        if(symbol==VOIDSY||symbol==INTSY||symbol==CHARSY) type=symbol;
        else{
            error(STX_ERR,TYPEERR);
            type=VOIDSY;
        }
        symbol = nextsym();//name
        if(symbol==IDSY){
            strcpy(name,idsy_value);
            symbol = nextsym();//( expected
            if(symbol!=LPARSY) error(STX_ERR,LPERR);
            symbol = nextsym();//paramlist started
            func(type,name);
        }
        else if(symbol==MAINSY){
            symbol = nextsym();
            func_main();
            break;
        }
        else error(STX_ERR,INVALID);
    }
    if((symbol = nextsym())){
        error(STX_ERR,SURPLUSERR);
    }
    emitjmain();
    if(debug) fprintf(fw,"This is a program\n");
    output_code4();
}

