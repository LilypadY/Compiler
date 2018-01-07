#include<stdio.h>
#include"err.h"
#include"memory.h"
#include"table.h"
int c4p = 1;//0 for j main
int optp = 1;

int tmp_var_cont = 0;
int label_cont = 0;
int labels[LBMAX];
int opt_labels[LBMAX];
int opt_label_cont = 0;
int strlp = 0;


int emit3(int f,int x,int y,int z,int flag_x,int flag_y){
    if(errs>0) return 0;
    code4[c4p].f = f;
    code4[c4p].x = x;
    code4[c4p].flag_x = flag_x;
    code4[c4p].y = y;
    code4[c4p].flag_y = flag_y;
    code4[c4p].z = z;
    switch(f){
        case ADD:case SUB:case MUL:case DIV:case GETA:code4[c4p].flag_z = F_VAR;break;
        case BEQ:case BNE:case BGE:case BGT:case BLT:case BLE: code4[c4p].flag_z = F_LBL;break;
        case ASNA:code4[c4p].flag_z = F_ARR;break;
    }
    return c4p++;
}
int emit2(int f,int x,int z,int flag_x,int flag_z){
    if(errs>0) return 0;
    code4[c4p].f = f;
    code4[c4p].x = x;
    code4[c4p].flag_x = flag_x;
    code4[c4p].z = z;
    code4[c4p].flag_z = flag_z;
    return c4p++;
}
int emit1(int f,int x,int flag_x){
    if(errs>0) return 0;
    code4[c4p].f = f;
    code4[c4p].x = x;
    code4[c4p].flag_x = flag_x;
    return c4p++;
}
int emit0(int f){
    if(errs>0) return 0;
    code4[c4p].f = f;
    return c4p++;
}
void emitjmain(){
    if(errs>0) return;
    int callmain = apply_label();
    code4[0].f = J;
    code4[0].flag_z = F_LBL;
    code4[0].z = callmain;
    emit1(CALL,bp,F_FUNC);
}
int apply_t(){
    if(errs>0) return -2;
    return tmp_var_cont++;
}
int apply_label(){
    if(errs>0) return -2;
    int i;
    for(i = 0;i < label_cont;i ++){
        if(labels[i]==c4p) return i;
    }
    labels[label_cont] = c4p;
    return label_cont++;
}
void fill_Border(int label){
    if(errs>0) return;
    code4[c4p-1].z = label;
    code4[c4p-1].flag_z = F_LBL;
}
void fill_Border_to(int label,int fcp){
    if(errs>0) return;
    if(fcp<0) return;
    code4[fcp].z = label;
    code4[fcp].flag_z = F_LBL;
}
void printcodex(int x,int flag_x){
        if(flag_x==F_VAL){
            fprintf(fw,"%d\t",x);
        }
        else if(flag_x==F_VAR){
            if(x>0)
                fprintf(fw,"v%d\t",x);
            else if(x==0)
                fprintf(fw,"GLV\t");
            else
                fprintf(fw,"G%d\t",-x);
        }
        else if(flag_x==F_LBL){
            fprintf(fw,"label%d\t",x);
        }
        else if(flag_x==F_FUNC){
            fprintf(fw,"%s:",tab[btab[x].head].name);
        }
        else if(flag_x==F_PARAM){
            fprintf(fw,"p%d\t",x);
        }
        else if(flag_x==F_STR){
            fprintf(fw,"%s\t",stringlist[x]);
        }
        else if(flag_x==F_ARR){
            if(x>0) fprintf(fw,"A%d\t",x);
            else fprintf(fw,"GA%d\t",-x);
        }
        else if(flag_x==F_TMPV){
            fprintf(fw,"t%d\t",x);
        }
}
char* code2str(int ord){
    char *str;
    switch(ord){
    case 	ADD 	:	str="	ADD 	";break;
	case 	SUB	    :	str="	SUB	    ";break;
	case 	MUL 	:	str="	MUL 	";break;
	case 	DIV 	:	str="	DIV 	";break;
	case 	GETA	:	str="	GETA	";break;
	case 	ASNA	:	str="	ASNA	";break;
	case 	BEQ 	:	str="	BEQ 	";break;
	case 	BNE 	:	str="	BNE 	";break;
	case 	BGE 	:	str="	BGE 	";break;
	case 	BGT 	:	str="	BGT 	";break;
	case 	BLT 	:	str="	BLT 	";break;
	case 	BLE 	:	str="	BLE     ";break;
	case 	J 	    :	str="	J       ";break;
	case 	CALL	:	str="	CALL	";break;
	case 	PUSH	:	str="	PUSH	";break;
	case 	RET	    :	str="	RET	    ";break;
	case 	RET0   	:	str="	RET0   	";break;
	case 	MOVE    :	str="	MOVE    ";break;
	case 	SYS_READCH    	:	str="	SYS_READCH    	";break;
	case 	SYS_READINT    	:	str="	SYS_READINT    	";break;
	case 	SYS_PRINTSTR    :	str="	SYS_PRINTSTR    ";break;
	case 	SYS_PRINTINT   	:	str="	SYS_PRINTINT   	";break;
	case 	SYS_PRINTCH    	:	str="	SYS_PRINTCH    	";break;
    }
    return str;
}

void output_code4(){
    int i,j=0,k = 1;
    fprintf(fw,"--------------\n\ncodes:\n");
    if(errs) return;
    for(i = 0;i < c4p;i ++){
        while(j<label_cont&&labels[j]==i){
            fprintf(fw,"label%d:",j++);
        }
        if(tab[btab[k].head].addr==i){
            fprintf(fw,"%s\t",tab[btab[k++].head].name);
        }
        fprintf(fw,"%s ",code2str(code4[i].f));
        printcodex(code4[i].x,code4[i].flag_x);
        printcodex(code4[i].y,code4[i].flag_y);
        printcodex(code4[i].z,code4[i].flag_z);
        fprintf(fw,"\n");
    }
}
int opt_emit(int f,int x,int y,int z,int flag_x,int flag_y,int flag_z){
    opt_code[optp].f = f;
    opt_code[optp].x = x;
    opt_code[optp].flag_x = flag_x;
    opt_code[optp].y = y;
    opt_code[optp].flag_y = flag_y;
    opt_code[optp].z = z;
    opt_code[optp].flag_z = flag_z;
    return optp++;

}
void output_opt_code4(){
    int i,j = 0;
    if(errs) return;
    if(debug){
        printf("labelcont=%d\n",opt_label_cont);
    }
    fprintf(fw,"----------------\n\n");
    for(i = 0;i < optp;i ++){
        if(j<opt_label_cont&&opt_labels[j]==i){
            fprintf(fw,"label%d:",j++);
        }
        fprintf(fw,"%s ",code2str(opt_code[i].f));
        printcodex(opt_code[i].x,opt_code[i].flag_x);
        printcodex(opt_code[i].y,opt_code[i].flag_y);
        printcodex(opt_code[i].z,opt_code[i].flag_z);
        fprintf(fw,"\n");
    }
    fprintf(fw,"----------------\n\n");
}
void fill_label(int line){
    int i,flag = 0;
    for(i = 0;i < label_cont;i ++){
        if(labels[i]==line){
            opt_labels[opt_label_cont++] = optp;
            break;
        }
    }
}
void opt_emitjmain(){
    if(errs>0) return;
    int callmain = label_cont-1;
    opt_code[0].f = J;
    opt_code[0].flag_z = F_LBL;
    opt_code[0].z = callmain;
    opt_emit(CALL,bp,0,0,F_FUNC,0,0);
}
