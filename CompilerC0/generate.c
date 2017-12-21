#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"compile.h"
#include"table.h"
#include"memory.h"
#include"err.h"

#define RA -1
#define V0 MAXREG
#define V1 -3
#define A0 0

int tmp_reg[MAXREG];
int func_idx;
int gl_arrs;

FILE* fp_code;

void print_newline();
void mips_data_seg();
char* regname(int reg);
int arrsize(int x);
int arrhead(int x);
void save_func_context();
void recover_reg_context();
int get_reg(int var);
int tmp_reg_apply(int var);
void tmp_reg_free(int tmp);
void load_glob_wd_to(int var,int reg);
void load_local_var_to(int var,int reg);
void store_global_var_to(int var,int reg);
void store_local_var_to(int var,int reg);
void init_global_memory();
void branch_act(int i,char *order);

void mips_gene(){
    //º¯ÊýÐÎ²ÎÖ»ÔÚmoveÖÐ³öÏÖ
    if(errs>0) return;
    fp_code = fopen("code.txt","w");
    mips_data_seg();

    int i,j,k = 1;
    struct loopblk* p=head->next;
    for(i = 0;i < c4p;i ++){
        for(j = 0;j < label_cont;j ++){
            if(labels[j]==i){
                fprintf(fp_code,"label%d:\n",j);
                if(j==label_cont-1) func_idx ++;
            }
        }
        if(tab[btab[k].head].addr==i){
            fprintf(fp_code,"%s:",tab[btab[k++].head].name);
            func_idx ++;
            for(j = 0;j < MAXREG;j ++) tmp_reg[j] = 0;
        }


        switch(code4[i].f){
            case ADD:{
                int rd;
                rd = get_reg(code4[i].z);
                if(rd<0){
                    rd = tmp_reg_apply(code4[i].z);
                }
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].y);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rs);
                        else load_glob_wd_to(code4[i].y,rs);
                    }
                    fprintf(fp_code,"addi $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].x);
                    if(rsf) tmp_reg_free(rs);

                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    fprintf(fp_code,"addi $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].y);
                    if(rsf) tmp_reg_free(rs);
                }
                else{
                    int rs,rt,rsf = 0,rtf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    rt = get_reg(code4[i].y);
                    if(rt<0){
                        rtf = 1;
                        rt = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rt);
                        else load_glob_wd_to(code4[i].y,rt);
                    }
                    fprintf(fp_code,"add $%s,$%s,$%s\n",regname(rd),regname(rs),regname(rt));
                    if(rsf) tmp_reg_free(rs);
                    if(rtf) tmp_reg_free(rt);
                }
                break;
                }
            case SUB:{
                int rd;
                rd = get_reg(code4[i].z);
                if(rd<0){
                    rd = tmp_reg_apply(code4[i].z);
                }
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].y);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rs);
                        else load_glob_wd_to(code4[i].y,rs);
                    }
                    fprintf(fp_code,"subi $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].x);
                    fprintf(fp_code,"sub $%s,$0,$%s\n",regname(rd),regname(rd));
                    if(rsf) tmp_reg_free(rs);
                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    fprintf(fp_code,"subi $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].y);
                    if(rsf) tmp_reg_free(rs);
                }
                else{
                    int rs,rt,rsf = 0,rtf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    rt = get_reg(code4[i].y);
                    if(rt<0){
                        rtf = 1;
                        rt = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rt);
                        else load_glob_wd_to(code4[i].y,rt);
                    }
                    fprintf(fp_code,"sub $%s,$%s,$%s\n",regname(rd),regname(rs),regname(rt));
                    if(rsf) tmp_reg_free(rs);
                    if(rtf) tmp_reg_free(rt);
                }
                break;
            }
            case MUL:{
                int rd;
                rd = get_reg(code4[i].z);
                if(rd<0){
                    rd = tmp_reg_apply(code4[i].z);
                }
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].y);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rs);
                        else load_glob_wd_to(code4[i].y,rs);
                    }
                    fprintf(fp_code,"mul $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].x);
                    if(rsf) tmp_reg_free(rs);

                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    fprintf(fp_code,"mul $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].y);
                    if(rsf) tmp_reg_free(rs);
                }
                else{
                    int rs,rt,rsf = 0,rtf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    rt = get_reg(code4[i].y);
                    if(rt<0){
                        rtf = 1;
                        rt = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rt);
                        else load_glob_wd_to(code4[i].y,rt);
                    }
                    fprintf(fp_code,"mul $%s,$%s,$%s\n",regname(rd),regname(rs),regname(rt));
                    if(rsf) tmp_reg_free(rs);
                    if(rtf) tmp_reg_free(rt);
                }
                break;
            }
            case DIV:{
                int rd;
                rd = get_reg(code4[i].z);
                if(rd<0){
                    rd = tmp_reg_apply(code4[i].z);
                }
                if(code4[i].flag_x==F_VAL&&code4[i].flag_y==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].y);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rs);
                        else load_glob_wd_to(code4[i].y,rs);
                    }
                    fprintf(fp_code,"li $%s,%d\n",regname(rd),code4[i].x);
                    fprintf(fp_code,"div $%s,$%s,$%s\n",regname(rd),regname(rd),regname(rs));
                    if(rsf) tmp_reg_free(rs);
                }
                else if(code4[i].flag_y==F_VAL&&code4[i].flag_x==F_VAR){
                    int rs,rsf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    fprintf(fp_code,"div $%s,$%s,%d\n",regname(rd),regname(rs),code4[i].y);
                    if(rsf) tmp_reg_free(rs);
                }
                else{
                    int rs,rt,rsf = 0,rtf = 0;
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rsf = 1;
                        rs = tmp_reg_apply(code4[i].x);
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                    rt = get_reg(code4[i].y);
                    if(rt<0){
                        rtf = 1;
                        rt = tmp_reg_apply(code4[i].y);
                        if(code4[i].y>0) load_local_var_to(code4[i].y,rt);
                        else load_glob_wd_to(code4[i].y,rt);
                    }
                    fprintf(fp_code,"div $%s,$%s,$%s\n",regname(rd),regname(rs),regname(rt));
                    if(rsf) tmp_reg_free(rs);
                    if(rtf) tmp_reg_free(rt);
                }
                break;
            }
            case MOVE:{
                int rd;
                if(code4[i].flag_z==F_VAR){
                    rd = get_reg(code4[i].z);
                    if(rd<0){
                        rd = tmp_reg_apply(code4[i].z);
                    }
                }
                else if(code4[i].flag_z==F_PARAM){
                    rd = V1;
                }

                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"li $%s,%d\n",regname(rd),code4[i].x);
                }
                else if(code4[i].flag_x==F_VAR) {
                    int rs = get_reg(code4[i].x);
                    if(rs<0){
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rd);
                        else load_glob_wd_to(code4[i].x,rd);
                    }
                    else{
                        fprintf(fp_code,"move $%s,$%s\n",regname(rd),regname(rs));
                    }
                }
                else if(code4[i].flag_x==F_PARAM){
                        fprintf(fp_code,"lw $%s,%d($fp)\n",regname(rd),(code4[i].x+1)*4);
                }
                if(code4[i].flag_z==F_PARAM)
                    fprintf(fp_code,"sw $v1,%d($fp)\n",code4[i].z*4);
                break;
            }
            case GETA:{//z=x[y]
                int res = get_reg(code4[i].z);
                if(res<0){
                    res = tmp_reg_apply(code4[i].z);
                }
                int headoff = arrhead(code4[i].x);
                char *base = (code4[i].x>0)?"fp":"gp";
                if(code4[i].flag_y==F_VAL){
                    fprintf(fp_code,"lw $%s,%d($%s)\n",regname(res),code4[i].y*4+headoff,base);
                }
                else{
                    int regy = get_reg(code4[i].y),yf=0;
                    if(regy<0){
                        regy = tmp_reg_apply(code4[i].y);
                        if(code4[i].y<0) load_glob_wd_to(code4[i].y,regy);
                        else load_local_var_to(code4[i].y,regy);
                        yf = 1;
                    }
                    fprintf(fp_code,"sll $%s,$%s,2\n",regname(res),regname(regy));
                    fprintf(fp_code,"add $%s,$%s,$%s\n",regname(res),regname(res),base);
                    fprintf(fp_code,"lw $%s,%d($%s)\n",regname(res),headoff,regname(res));
                    if(yf) tmp_reg_free(regy);
                }
                break;
            }
            case ASNA:{//z[x]=y
                int rd,rdf = 0;
                if(code4[i].flag_y==F_VAL){
                    rd =  tmp_reg_apply(btab[func_idx].vd);
                    fprintf(fp_code,"li $%s,%d\n",regname(rd),code4[i].y);
                    rdf = 1;
                }
                else{
                    rd = get_reg(code4[i].y);
                    if(rd<0){
                        rd =  tmp_reg_apply(btab[func_idx].vd);
                        if(code4[i].y<0) store_global_var_to(code4[i].y,rd);
                        else store_local_var_to(code4[i].y,rd);
                        rdf = 1;
                    }
                }
                int headoff = arrhead(code4[i].z);
                char *base = (code4[i].z>0)?"fp":"gp";

                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"sw $%s,%d($%s)\n",regname(rd),code4[i].x*4+headoff,base);
                }
                else{
                    int regx = get_reg(code4[i].x);
                    if(regx<0){
                        regx = tmp_reg_apply(code4[i].x);
                        if(code4[i].x<0) load_glob_wd_to(code4[i].x,regx);
                        else load_local_var_to(code4[i].x,regx);
                    }
                    fprintf(fp_code,"sll $%s,$%s,2\n",regname(regx),regname(regx));
                    fprintf(fp_code,"add $%s,$%s,$%s\n",regname(regx),regname(regx),base);
                    fprintf(fp_code,"sw $%s,%d($%s)\n",regname(rd),headoff,regname(regx));
                    tmp_reg_free(regx);
                }
                if(rdf) tmp_reg_free(rd);
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
                int rs,rsf = 0;
                if(code4[i].flag_x==F_VAL){
                    rs = tmp_reg_apply(btab[func_idx].vd);
                    fprintf(fp_code,"li $%s,%d\n",regname(rs),code4[i].x);
                    rsf = 1;
                }
                else{
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        rs = tmp_reg_apply(code4[i].x);
                        rsf = 1;
                        if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
                        else load_glob_wd_to(code4[i].x,rs);
                    }
                }
                fprintf(fp_code,"sw $%s,%d($sp)\n",regname(rs),code4[i].z*4);
                if(rsf) tmp_reg_free(rs);
                break;
            }
            case RET:{
                int rs;
                if(code4[i].flag_x==F_VAL){
                    fprintf(fp_code,"li $v0,%d\n",code4[i].x);
                }
                else{
                    rs = get_reg(code4[i].x);
                    if(rs<0){
                        if(code4[i].x>0) load_local_var_to(code4[i].x,V0);
                        else load_glob_wd_to(code4[i].x,V0);
                    }
                    else{
                        if(code4[i].x!=0) fprintf(fp_code,"move $v0,$%s\n",regname(rs));
                    }
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
                if(tmp_reg[0]){
                    if(code4[i].x>0) store_local_var_to(tmp_reg[0],A0);
                    else store_local_var_to(tmp_reg[0],A0);
                    tmp_reg[0] = 0;
                }
                if(code4[i].flag_x==F_VAL) fprintf(fp_code,"li $a0,%d\n",code4[i].x);
                else{
                    int rs = get_reg(code4[i].x);
                    if(rs>=0) fprintf(fp_code,"move $a0,$%s\n",regname(rs));
                    else{
                        if(code4[i].x>0) load_local_var_to(code4[i].x,A0);
                        else load_glob_wd_to(code4[i].x,A0);
                    }
                }
                fprintf(fp_code,"li $v0,%d\n",1);
                fprintf(fp_code,"syscall\n");
                print_newline();
                break;
            }
            case SYS_PRINTCH:{
                if(tmp_reg[0]){
                    if(code4[i].x>0) store_local_var_to(tmp_reg[0],A0);
                    else store_global_var_to(tmp_reg[0],A0);
                    tmp_reg[0] = 0;
                }
                if(code4[i].flag_x==F_VAL) fprintf(fp_code,"li $a0,%d\n",code4[i].x);
                else{
                    int rs = get_reg(code4[i].x);
                    if(rs>=0) fprintf(fp_code,"move $a0,$%s\n",regname(rs));
                    else{
                        if(code4[i].x>0) load_local_var_to(code4[i].x,A0);
                        else load_glob_wd_to(code4[i].x,A0);
                    }
                }
                fprintf(fp_code,"li $v0,%d\n",11);
                fprintf(fp_code,"syscall\n");
                print_newline();
                break;
            }
            case SYS_PRINTSTR:{
                if(tmp_reg[0]){
                    if(code4[i].x>0) store_local_var_to(tmp_reg[0],A0);
                    else store_global_var_to(tmp_reg[0],A0);
                    tmp_reg[0] = 0;
                }
                fprintf(fp_code,"la $a0,str%d\n",code4[i].x);
                fprintf(fp_code,"li $v0,%d\n",4);
                fprintf(fp_code,"syscall\n");
                print_newline();
                break;
            }
            case SYS_READCH:{
                if(tmp_reg[0]){
                    if(code4[i].x>0) store_local_var_to(tmp_reg[0],A0);
                    else store_global_var_to(tmp_reg[0],A0);
                    tmp_reg[0] = 0;
                }
                fprintf(fp_code,"li $v0,%d\n",12);
                fprintf(fp_code,"syscall\n");
                break;
            }
            case SYS_READINT:{
                if(tmp_reg[0]){
                    if(code4[i].x>0) store_local_var_to(tmp_reg[0],A0);
                    else store_global_var_to(tmp_reg[0],A0);
                    tmp_reg[0] = 0;
                }
                fprintf(fp_code,"li $v0,%d\n",5);
                fprintf(fp_code,"syscall\n");
                break;
            }
        }
    }
    fclose(fp_code);
}
char* regname(int reg){
    char *s=(char*)malloc(sizeof(int)*2);
    if(reg==V0){
        strcpy(s,"v0");
    }
    else if(reg==V1){
        strcpy(s,"v1");
    }
    else if(reg==RA){
        strcpy(s,"ra");
    }
    else{
        sprintf(s,"%d",reg+4);
    }
    return s;
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
    return (arrp+off+btab[func_idx].params)*4;
}
void save_func_context(){
    int j;
    if(func_idx<=bp) fprintf(fp_code,"sw $ra,0($fp)\n");
    for(j = 0;j < MAXREG;j ++){
        if(tmp_reg[j]){
            if(tmp_reg[j]<0) store_global_var_to(tmp_reg[j],j);
            else store_local_var_to(tmp_reg[j],j);
        }
    }
}
void recover_reg_context(){
    int j;
    if(func_idx<=bp) fprintf(fp_code,"lw $ra,0($fp)\n");
    for(j = 0;j < MAXREG;j ++){
        if(tmp_reg[j]){
            if(tmp_reg[j]<0) load_glob_wd_to(tmp_reg[j],j);
            else load_local_var_to(tmp_reg[j],j);
        }
    }
}
int get_reg(int var){
    int i;
    if(var==0) return V0;
    for(i = 0;i < MAXREG; i ++){
        if(tmp_reg[i]==var){
            return i;
        }
    }
    return -1;
}
void load_local_var_to(int var,int reg){
    fprintf(fp_code,"lw $%s,%d($sp)\n",regname(reg),(var-btab[func_idx].vd)*4);
}
void load_glob_wd_to(int var,int reg){
    fprintf(fp_code,"lw $%s,%d($gp)\n",regname(reg),(-var-1+gl_arrs)*4);
}
void store_global_var_to(int var,int reg){
    fprintf(fp_code,"sw $%s,%d($gp)\n",regname(reg),(-var-1+gl_arrs)*4);
}
void store_local_var_to(int var,int reg){
    fprintf(fp_code,"sw $%s,%d($sp)\n",regname(reg),(var-btab[func_idx].vd)*4);
}
int tmp_reg_apply(int var){
    static int spa = 0;
    int i,k;
    if((k = get_reg(var))>=0) return k;
    for(i = 0;i < MAXREG; i ++){
        if(!tmp_reg[i]){
            tmp_reg[i] = var;
            return i;
        }
    }
    if(spa==MAXREG) spa = 0;
    if(tmp_reg[spa]<0) store_global_var_to(tmp_reg[spa],spa);
    else store_local_var_to(tmp_reg[spa],spa);
    tmp_reg[spa]=var;
    return spa ++;
}

void tmp_reg_free(int tmp){
    if(tmp<0) {
            error(MEMORY_ERR,NOMORE);
    }
    else if(tmp<MAXREG){
        tmp_reg[tmp] = 0;
    }
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
    int i,mem=1;
    mem += btab[0].vd-1;
    for(i = 1;i < btab[0].ap; i++){
        mem += btab[0].arrays[i].len;
    }
    fprintf(fp_code,"addi $sp,$sp,%d\n",mem*4);
    func_idx = 0;
}
void branch_act(int i,char *order){
    int rs,rt,rsf=0,rtf=0;
    if(code4[i].flag_x==F_VAL){
        rs = tmp_reg_apply(btab[func_idx].vd);
        rsf = 1;
        fprintf(fp_code,"li $%s,%d\n",regname(rs),code4[i].x);
    }
    else{
        rs = get_reg(code4[i].x);
        if(rs<0){
            rs = tmp_reg_apply(btab[func_idx].vd);
            rsf = 1;
            if(code4[i].x>0) load_local_var_to(code4[i].x,rs);
            else load_glob_wd_to(code4[i].x,rs);
        }
    }
    if(code4[i].flag_y==F_VAL){
        rt = tmp_reg_apply(btab[func_idx].vd+1);
        rtf = 1;
        fprintf(fp_code,"li $%s,%d\n",regname(rt),code4[i].y);
    }
    else{
        rt = get_reg(code4[i].y);
        if(rt<0){
            rt = tmp_reg_apply(btab[func_idx].vd+1);
            rtf = 1;
            if(code4[i].x>0) load_local_var_to(code4[i].y,rt);
            else load_glob_wd_to(code4[i].y,rt);
        }
    }
    fprintf(fp_code,"%s $%s,$%s,label%d\n",order,regname(rs),regname(rt),code4[i].z);
    if(rsf) tmp_reg_free(rs);
    if(rtf) tmp_reg_free(rt);
}
void print_newline(){
    fprintf(fp_code,"la $a0,strnl\n");
    fprintf(fp_code,"li $v0,%d\n",4);
    fprintf(fp_code,"syscall\n");
}
