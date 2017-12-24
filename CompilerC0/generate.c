#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"compile.h"
#include"table.h"
#include"memory.h"
#include"err.h"

#define RA -1
#define V0 2
#define A0 4

#define RS 8
#define RT 9
#define RD 10

int func_idx;
int gl_arrs;

FILE* fp_code;

int imrw = 1;

void print_newline();
void mips_data_seg();
int arrsize(int x);
int arrhead(int x);
void save_func_context();
void recover_reg_context();
void load_var_to(int var,int reg);
void store_var_to(int var,int reg);
void init_global_memory();
void branch_act(int i,char *order);
void load_param_to(int var,int reg);
void store_param_to(int var,int reg);

void mips_gene(){
    //函数形参只在move中出现
    if(errs>0) return;
    fp_code = fopen("code.txt","w");
    mips_data_seg();

    int i,j,k = 1;
    //struct loopblk* p=head->next;
    for(i = 0;i < c4p;i ++){
        if(tab[btab[k].head].addr==i){
            fprintf(fp_code,"%s:\n",tab[btab[k++].head].name);
            func_idx ++;
        }
        for(j = 0;j < label_cont;j ++){
            if(labels[j]==i){
                fprintf(fp_code,"label%d:\n",j);
                if(j==label_cont-1) func_idx ++;
            }
        }
        switch(code4[i].f){
            case ADD:{
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"addi $%d,$%d,%d\n",RD,RT,code4[i].x);
                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    load_var_to(code4[i].x,RS);
                    fprintf(fp_code,"addi $%d,$%d,%d\n",RD,RS,code4[i].y);
                }
                else{
                    load_var_to(code4[i].x,RS);
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"add $%d,$%d,$%d\n",RD,RS,RT);
                }
                if(imrw){
                    if(code4[i].z) store_var_to(code4[i].z,RD);
                }
                break;
                }
            case SUB:{
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"subi $%d,$%d,%d\n",RD,RT,code4[i].x);
                    fprintf(fp_code,"sub $%d,$0,$%d\n",RD,RD);
                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    load_var_to(code4[i].x,RS);
                    fprintf(fp_code,"subi $%d,$%d,%d\n",RD,RS,code4[i].y);
                }
                else{
                    load_var_to(code4[i].x,RS);
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"sub $%d,$%d,$%d\n",RD,RS,RT);
                }
                if(imrw){
                    if(code4[i].z) store_var_to(code4[i].z,RD);
                }
                break;
            }
            case MUL:{
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"mul $%d,$%d,%d\n",RD,RT,code4[i].x);
                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    load_var_to(code4[i].x,RS);
                    fprintf(fp_code,"mul $%d,$%d,%d\n",RD,RS,code4[i].y);
                }
                else{
                    load_var_to(code4[i].x,RS);
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"mul $%d,$%d,$%d\n",RD,RS,RT);
                }
                if(imrw){
                    if(code4[i].z) store_var_to(code4[i].z,RD);
                }
                break;
            }
            case DIV:{
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"li $%d,%d\n",RS,code4[i].x);
                    fprintf(fp_code,"div $%d,$%d,$%d\n",RD,RS,RT);
                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    load_var_to(code4[i].x,RS);
                    fprintf(fp_code,"div $%d,$%d,%d\n",RD,RD,code4[i].y);
                }
                else{
                    load_var_to(code4[i].x,RS);
                    load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"div $%d,$%d,$%d\n",RD,RS,RT);
                }
                if(imrw){
                    if(code4[i].z) store_var_to(code4[i].z,RD);
                }
                break;
            }
            case MOVE:{
                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",RD,code4[i].x);
                }
                else if(code4[i].flag_x==F_VAR) {
                    load_var_to(code4[i].x,RD);
                }
                else if(code4[i].flag_x==F_PARAM){
                    load_param_to(code4[i].x,RD);
                }
                if(code4[i].flag_z==F_PARAM)
                    store_param_to(code4[i].z,RD);
                else if(code4[i].flag_z==F_VAR){
                    store_var_to(code4[i].z,RD);
                }
                break;
            }
            case GETA:{//z=x[y]
                int headoff = arrhead(code4[i].x);
                char *base = (code4[i].x>0)?"fp":"gp";
                if(code4[i].flag_y==F_VAL){
                    fprintf(fp_code,"lw $%d,%d($%s)\n",RD,code4[i].y*4+headoff,base);
                }
                else{
                    if(code4[i].flag_y==F_PARAM)
                        store_param_to(code4[i].y,RT);
                    else if(code4[i].flag_y==F_VAR)
                        load_var_to(code4[i].y,RT);
                    fprintf(fp_code,"sll $%d,$%d,2\n",RD,RT);
                    fprintf(fp_code,"add $%d,$%d,$%s\n",RD,RD,base);
                    fprintf(fp_code,"lw $%d,%d($%d)\n",RD,headoff,RD);
                }
                if(imrw){
                    if(code4[i].flag_z==F_PARAM)
                        store_param_to(code4[i].z,RD);
                    else if(code4[i].flag_z==F_VAR){
                        store_var_to(code4[i].z,RD);
                    }
                }
                break;
            }
            case ASNA:{//z[x]=y
                if(code4[i].flag_y==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",RT,code4[i].y);
                }
                else if(code4[i].flag_y==F_VAR) {
                    load_var_to(code4[i].y,RT);
                }
                else if(code4[i].flag_y==F_PARAM){
                    load_param_to(code4[i].y,RT);
                }
                int headoff = arrhead(code4[i].z);
                char *base = (code4[i].z>0)?"fp":"gp";

                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"sw $%d,%d($%s)\n",RT,code4[i].x*4+headoff,base);
                }
                else {
                    if(code4[i].flag_x==F_VAR) {
                        load_var_to(code4[i].x,RS);
                    }
                    else if(code4[i].flag_x==F_PARAM){
                        load_param_to(code4[i].x,RS);
                    }
                    fprintf(fp_code,"sll $%d,$%d,2\n",RS,RS);
                    fprintf(fp_code,"add $%d,$%d,$%s\n",RS,RS,base);
                    fprintf(fp_code,"sw $%d,%d($%d)\n",RT,headoff,RS);
                }
                break;
            }
            case CALL:{
                save_func_context();
                int nsp = 1+btab[code4[i].x].params+arrsize(code4[i].x)+btab[code4[i].x].vd-1;
                fprintf(fp_code,"move $fp,$sp\n");
                fprintf(fp_code,"addi $sp,$sp,%d\n",nsp*4);
                fprintf(fp_code,"jal %s\n",tab[btab[code4[i].x].head].name);
                int osp = 1+btab[func_idx].params+arrsize(func_idx)+btab[func_idx].vd-1;
                if(func_idx==bp+1) osp--;
                fprintf(fp_code,"subi $fp,$fp,%d\n",osp*4);
                recover_reg_context();
                break;
            }
            case PUSH:{
                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",RS,code4[i].x);
                }
                else if(code4[i].flag_x==F_VAR) {
                    load_var_to(code4[i].x,RS);
                }
                else if(code4[i].flag_x==F_PARAM){
                    load_param_to(code4[i].x,RS);
                }
                fprintf(fp_code,"sw $%d,%d($sp)\n",RS,(code4[i].z+1)*4);
                break;
            }
            case RET:{
                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",RD,code4[i].x);
                }
                else if(code4[i].flag_x==F_VAR) {
                    load_var_to(code4[i].x,RD);
                }
                else if(code4[i].flag_x==F_PARAM){
                    load_param_to(code4[i].x,RD);
                }
                fprintf(fp_code,"move $sp,$fp\n");
                fprintf(fp_code,"jr $ra\n");
                break;
            }
            case RET0:{
                fprintf(fp_code,"move $sp,$fp\n");
                fprintf(fp_code,"jr $ra\n");
                break;
            }
            case BEQ:branch_act(i,"beq");break;
            case BNE:branch_act(i,"bne");break;
            case BLE:branch_act(i,"ble");break;
            case BGE:branch_act(i,"bge");break;
            case BGT:branch_act(i,"bgt");break;
            case BLT:branch_act(i,"blt");break;
            case J:fprintf(fp_code,"j label%d\n",code4[i].z);break;
            case SYS_PRINTINT:{
                if(code4[i].flag_x==F_VAL) fprintf(fp_code,"li $a0,%d\n",code4[i].x);
                else if(code4[i].flag_x==F_VAR) {
                    load_var_to(code4[i].x,A0);
                }
                else if(code4[i].flag_x==F_PARAM){
                    load_param_to(code4[i].x,A0);
                }
                fprintf(fp_code,"li $v0,%d\n",1);
                fprintf(fp_code,"syscall\n");
                break;
            }
            case SYS_PRINTCH:{
                if(code4[i].flag_x==F_VAL) fprintf(fp_code,"li $a0,%d\n",code4[i].x);
                else if(code4[i].flag_x==F_VAR) {
                    load_var_to(code4[i].x,A0);
                }
                else if(code4[i].flag_x==F_PARAM){
                    load_param_to(code4[i].x,A0);
                }
                fprintf(fp_code,"li $v0,%d\n",11);
                fprintf(fp_code,"syscall\n");
                break;
            }
            case SYS_PRINTSTR:{
                if(code4[i].x==-1) print_newline();
                else{
                    fprintf(fp_code,"la $a0,str%d\n",code4[i].x);
                    fprintf(fp_code,"li $v0,%d\n",4);
                    fprintf(fp_code,"syscall\n");
                }
                break;
            }
            case SYS_READCH:{
                fprintf(fp_code,"li $v0,%d\n",12);
                fprintf(fp_code,"syscall\n");
                fprintf(fp_code,"move $%d,$v0\n",RD);
                break;
            }
            case SYS_READINT:{
                fprintf(fp_code,"li $v0,%d\n",5);
                fprintf(fp_code,"syscall\n");
                fprintf(fp_code,"move $%d,$v0\n",RD);
                break;
            }
        }
    }
    fclose(fp_code);
}

int arrsize(int x){
    int arrp = 0;
    int j;
    for(j = 1;j < btab[x].ap;j ++){
        arrp += btab[x].arrays[j].len;
    }
    return arrp;
}
int arrhead(int arridx){//by bottom
    int arrp = 0,off,x;
    int j;
    if(arridx>0){
        off = 1;
        x = func_idx;
    }
    else{
        off = 0;
        x = 0;
    }
    arridx = abs(arridx);
    for(j = 1;j < arridx;j ++) arrp += btab[x].arrays[j].len;
    return (arrp+off+btab[x].params)*4;
}
void save_func_context(){
    if(func_idx<=bp) fprintf(fp_code,"sw $ra,0($fp)\n");
}
void recover_reg_context(){
    if(func_idx<=bp) fprintf(fp_code,"lw $ra,0($fp)\n");
}
void load_local_var_to(int var,int reg){
    int asize = arrsize(func_idx);
    int psize = btab[func_idx].params;
    int s = asize+psize+1;
    fprintf(fp_code,"lw $%d,%d($fp)\n",reg,(var-1+s)*4);
}
void load_glob_wd_to(int var,int reg){
    fprintf(fp_code,"lw $%d,%d($gp)\n",reg,(-var-1+gl_arrs)*4);
}
void store_global_var_to(int var,int reg){
    fprintf(fp_code,"sw $%d,%d($gp)\n",reg,(-var-1+gl_arrs)*4);
}
void store_local_var_to(int var,int reg){
    int asize = arrsize(func_idx);
    int psize = btab[func_idx].params;
    int s = asize+psize+1;
    fprintf(fp_code,"sw $%d,%d($fp)\n",reg,(var-1+s)*4);
}
void load_param_to(int var,int reg){
    fprintf(fp_code,"lw $%d,%d($fp)\n",reg,(var+1)*4);
}
void store_param_to(int var,int reg){
    fprintf(fp_code,"sw $%d,%d($fp)\n",reg,(var+1)*4);
}
void load_var_to(int var,int reg){
    if(var==0){
        if(reg!=RD){
            fprintf(fp_code,"move $%d,$%d\n",reg,RD);
        }
    }
    else if(var<0) load_glob_wd_to(var,reg);
    else load_local_var_to(var,reg);
}
void store_var_to(int var,int reg){
    if(var==0){
        if(reg!=RD){
            fprintf(fp_code,"move $%d,$%d\n",RD,reg);
        }
    }
    else if(var<0) store_global_var_to(var,reg);
    else store_local_var_to(var,reg);
}

void mips_data_seg(){
    int i,j,k;
    char strbuf[MAX_STR_LEN*2];
    gl_arrs = arrsize(0);
    fprintf(fp_code,".data\n");
    fprintf(fp_code,"strnl:.asciiz \"\\n\"\n");
    for(i = 0;i < strlp;i ++){
        k = 0;
        for(j = 0;;j ++){
            strbuf[k++] = stringlist[i][j];
            if(stringlist[i][j]=='\\'){
                strbuf[k++]='\\';
            }
            if(stringlist[i][j]=='\0'){
                break;
            }
        }
        fprintf(fp_code,"str%d:.asciiz \"%s\"\n",i,strbuf);

    }
    fprintf(fp_code,"global_start:.word 0\n");
    fprintf(fp_code,".text\n");
    fprintf(fp_code,"la $sp,global_start\n");
    fprintf(fp_code,"move $fp,$sp\n");
    fprintf(fp_code,"move $gp,$sp\n");
    init_global_memory();
}
void init_global_memory(){
    int i,mem=0;
    mem += btab[0].vd-1;
    for(i = 1;i < btab[0].ap; i++){
        mem += btab[0].arrays[i].len;
    }
    fprintf(fp_code,"addi $sp,$sp,%d\n",mem*4);
    func_idx = 0;
}
void branch_act(int i,char *order){
    if(code4[i].flag_x==F_VAL){
        fprintf(fp_code,"li $%d,%d\n",RS,code4[i].x);
    }
    else if(code4[i].flag_x==F_VAR) {
        load_var_to(code4[i].x,RS);
    }
    else if(code4[i].flag_x==F_PARAM){
        load_param_to(code4[i].x,RS);
    }
    if(code4[i].flag_y==F_VAL){
        fprintf(fp_code,"li $%d,%d\n",RT,code4[i].y);
    }
    else if(code4[i].flag_y==F_VAR) {
        load_var_to(code4[i].y,RT);
    }
    else if(code4[i].flag_y==F_PARAM){
        load_param_to(code4[i].y,RT);
    }
    fprintf(fp_code,"%s $%d,$%d,label%d\n",order,RS,RT,code4[i].z);
}
void print_newline(){
    fprintf(fp_code,"la $a0,strnl\n");
    fprintf(fp_code,"li $v0,%d\n",4);
    fprintf(fp_code,"syscall\n");
}


