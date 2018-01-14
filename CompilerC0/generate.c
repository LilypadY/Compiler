#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"compile.h"
#include"table.h"
#include"memory.h"
#include"err.h"
#include"optimize.h"

#define RA -1
#define V0 2
#define A0 4

#define RS 26
#define RT 27
#define RD 3


FILE* fp_code;

int func_idx;
int gl_arrs;

int imrw = 1;

void print_newline();
void mips_data_seg();
int arrsize(int x);
int arrhead(int x);
void save_func_context();
void recover_func_context();
void load_var_to(int var,int reg);
void store_var_to(int var,int reg);
void init_global_memory();
void branch_act(int i,char *order);
void load_param_to(int var,int reg);
void store_param_to(int var,int reg);
int getReg(int var,int flag,int place);

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
                recover_func_context();
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

void save_func_context(){
    if(func_idx<=bp) fprintf(fp_code,"sw $ra,0($fp)\n");
}
void save_reg_context(){
int i,j;
    if(func_idx<=bp){
        for(j = 0;j < btab[func_idx].params;j ++){
        if(j==MAXPARAMREG)
            break;
        store_param_to(j,PARAMREG+j);
    }
    for(i = 0;i < MAXV;i ++){
        if(regs[i]>0&&regocc[regs[i]]==1){
            if(i<btab[func_idx].vd-1){
                store_local_var_to(i+1,regs[i]-1+SREGST);
            }
            else{
                store_global_var_to(getGLAOrg(i+1),regs[i]-1+SREGST);
            }
        }
    }
    }
}
void recover_func_context(){
    if(func_idx<=bp) fprintf(fp_code,"lw $ra,0($fp)\n");
}
void recover_reg_context(){
    int i,j;
    if(func_idx<=bp){
        for(j = 0;j < btab[func_idx].params;j ++){
        if(j==MAXPARAMREG)
            break;
        load_param_to(j,PARAMREG+j);
    }
    for(i = 0;i < MAXV;i ++){
        if(regs[i]>0&&regocc[regs[i]]==1){
            if(i<btab[func_idx].vd-1){
                load_local_var_to(i+1,regs[i]-1+SREGST);
            }
            else{
                load_glob_wd_to(getGLAOrg(i+1),regs[i]-1+SREGST);
            }
        }
    }
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
void branch_act_new(int i,char *order){
    int x,y;
    if(opt_code[i].flag_x==F_VAL){
        fprintf(fp_code,"li $%d,%d\n",RS,opt_code[i].x);
        x = RS;
    }
    else {
        x = getReg(opt_code[i].x,opt_code[i].flag_x,RS);
    }
    if(opt_code[i].flag_y==F_VAL){
        fprintf(fp_code,"li $%d,%d\n",RT,opt_code[i].y);
        y = RT;
    }
    else{
        y = getReg(opt_code[i].y,opt_code[i].flag_y,RT);
    }
    fprintf(fp_code,"%s $%d,$%d,label%d\n",order,x,y,opt_code[i].z);
}
void print_newline(){
    fprintf(fp_code,"la $a0,strnl\n");
    fprintf(fp_code,"li $v0,%d\n",4);
    fprintf(fp_code,"syscall\n");
}

void opt_mips_data_seg(){
    int i,j,k;
    char strbuf[MAX_STR_LEN*2];

    fp_code = fw_opt_mips;

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
    fprintf(fp_code,"addi $sp,$sp,%d\n",MAX_FUNC_MEM*4);//global mem
}

void load_wd(int var){
    int reg = V0;
    if(var>0){
        if(regs[var-1]==-1) return;
        reg = regs[var-1]-1+SREGST;
        load_local_var_to(var,reg);
    }
    else if(var<0){
        reg = regs[getGLANew(var)-1];
        if(reg==-1) return;
        reg+=-1+SREGST;
        load_glob_wd_to(var,reg);
    }
}
void store_wd(int var){
    int reg = RD;
    if(var>0){
        if(regs[var-1]==-1) return;
        reg = regs[var-1]-1+SREGST;
        store_local_var_to(var,reg);
    }
    else if(var<0){
        reg = regs[getGLANew(var)-1];
        if(reg==-1) return;
        reg+=-1+SREGST;
        store_global_var_to(var,reg);
    }
}
void load_tmp_var_to(int var,int reg){
    fprintf(fp_code,"lw $%d,%d($sp)\n",reg,var*4);
}
void store_tmp_var_to(int var,int reg){
    fprintf(fp_code,"sw $%d,%d($sp)\n",reg,var*4);
}
int getReg(int var,int flag,int place){
    if(flag==F_VAR){
        if(var==0)
            return V0;
        else if(var>0){
            if(regs[var-1]>0)
                return regs[var-1]-1+SREGST;
            else{
                load_local_var_to(var,place);
                return place;
            }
        }
        else {
            int n = getGLANew(var);
            if(regs[n-1]>0){
                return regs[n-1]-1+SREGST;
            }
            else{
                load_glob_wd_to(var,place);
                return place;
            }
        }
    }
    else if(flag==F_PARAM){
        if(var<MAXPARAMREG)
            return PARAMREG+var;
        else{
            load_param_to(var,place);
            return place;
        }
    }
    else if(flag==F_TMPV){
        int treg = get_tv_reg(var);
        if(treg<MAXTREG)
            return treg+TREGST;
        else{
            load_tmp_var_to(treg-MAXTREG,place);
            return place;
        }
    }
    return 0;
}
void store(int x,int flag,int reg){
    if(flag==F_VAR){
        if(x>0){
            store_local_var_to(x,reg);
        }
        else {
            store_global_var_to(x,reg);
        }
    }
    else if(flag==F_PARAM){
        store_param_to(x,reg);
    }
    else if(flag==F_TMPV){
        store_tmp_var_to(x,reg);
    }
}
void opt_mips_gene(int btidx){
    int i,j;
    int x,y,z;
    struct udv_ *p;
    if(errs>0) return;
    createConfGraph(btidx);
    dist_SRegs();
    fprintf(fp_code,"%s:\n",tab[btab[btidx].head].name);
    func_idx = btidx;

    for(j = 0;j < btab[btidx].params;j ++){
        if(j==MAXPARAMREG)
            break;
        load_param_to(j,PARAMREG+j);
    }
    for(i = 0;i < MAXV;i ++){
        if(regs[i]>0&&regocc[regs[i]]==1){
            if(i<btab[btidx].vd-1){
                load_local_var_to(i+1,regs[i]-1+SREGST);
            }
            else{
                load_glob_wd_to(getGLAOrg(i+1),regs[i]-1+SREGST);
            }
        }
    }
    for(i = 0;i < bsc_blks;i ++){
        //init block vars
        dist_tmp_reg(i);
        for(j = 0;j < opt_label_cont;j ++){
            if(opt_labels[j]==bblocklist[i].opt_st){
                fprintf(fp_code,"label%d:\n",j);
                break;
            }
        }
        p = ins[i]->next;
        while(p){
            j = p->p->var;
            if(p->p->type==F_VAR){
                if(j>0&&regocc[regs[j-1]]>1||j<0&&regocc[regs[getGLANew(j)-1]]>1){
                        if(debug) printf("exc:%d\n",j);
                    load_wd(j);
                }
            }
            p = p->next;
        }

        for(j = bblocklist[i].opt_st;j <= bblocklist[i].opt_en;j ++){
                if(debug){
                    printf("f=%d\n",opt_code[j].f);
                }
            switch(opt_code[j].f){
                case ADD:{
                z = getReg(opt_code[j].z,opt_code[j].flag_z,RD);
                if(opt_code[j].flag_x==F_VAL&&opt_code[j].flag_y!=F_VAL){
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"addi $%d,$%d,%d\n",z,y,opt_code[j].x);
                }
                else if(opt_code[j].flag_y==F_VAL&&opt_code[j].flag_x!=F_VAL){
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    fprintf(fp_code,"addi $%d,$%d,%d\n",z,x,opt_code[j].y);
                }
                else{
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"add $%d,$%d,$%d\n",z,x,y);
                }
                if(z==RD){
                    store(z,opt_code[j].flag_z,RD);
                }
                break;
                }
            case SUB:{
                z = getReg(opt_code[j].z,opt_code[j].flag_z,RD);
                if(opt_code[j].flag_x==F_VAL&&opt_code[j].flag_y!=F_VAL){
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"subi $%d,$%d,%d\n",z,y,opt_code[j].x);
                    fprintf(fp_code,"sub $%d,$0,$%d\n",z,z);
                }
                else if(opt_code[j].flag_y==F_VAL&&opt_code[j].flag_x!=F_VAL){
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    fprintf(fp_code,"subi $%d,$%d,%d\n",z,x,opt_code[j].y);
                }
                else{
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"sub $%d,$%d,$%d\n",z,x,y);
                }
                if(z==RD){
                    store(z,opt_code[j].flag_z,RD);
                }
                break;
            }
            case MUL:{
                z = getReg(opt_code[j].z,opt_code[j].flag_z,RD);
                if(opt_code[j].flag_x==F_VAL&&opt_code[j].flag_y!=F_VAL){
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"mul $%d,$%d,%d\n",z,y,opt_code[j].x);
                }
                else if(opt_code[j].flag_y==F_VAL&&opt_code[j].flag_x!=F_VAL){
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    fprintf(fp_code,"mul $%d,$%d,%d\n",z,x,opt_code[j].y);
                }
                else{
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"mul $%d,$%d,$%d\n",z,x,y);
                }
                if(z==RD){
                    store(z,opt_code[j].flag_z,RD);
                }
                break;
            }
            case DIV:{
                z = getReg(opt_code[j].z,opt_code[j].flag_z,RD);
                if(opt_code[j].flag_x==F_VAL&&opt_code[j].flag_y!=F_VAL){
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"li $%d,%d\n",y,opt_code[j].x);
                    fprintf(fp_code,"div $%d,$%d,$%d\n",z,x,y);
                }
                else if(opt_code[j].flag_y==F_VAL&&opt_code[j].flag_x!=F_VAL){
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    fprintf(fp_code,"div $%d,$%d,%d\n",z,x,opt_code[j].y);
                }
                else{
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"div $%d,$%d,$%d\n",z,x,y);
                }
                if(z==RD){
                    store(z,opt_code[j].flag_z,RD);
                }
                break;
            }
            case MOVE:{
                z = getReg(opt_code[j].z,opt_code[j].flag_z,RD);
                if(opt_code[j].flag_x==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",z,opt_code[j].x);
                }
                else {
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                    fprintf(fp_code,"move $%d,$%d\n",z,x);
                }
                if(z==RD){
                    store(z,opt_code[j].flag_z,RD);
                }
                break;
            }
            case GETA:{//z=x[y]
                z = getReg(opt_code[j].z,opt_code[j].flag_z,RD);
                int headoff = arrhead(opt_code[j].x);
                char *base = (opt_code[j].x>0)?"fp":"gp";
                if(opt_code[j].flag_y==F_VAL){
                    fprintf(fp_code,"lw $%d,%d($%s)\n",z,opt_code[j].y*4+headoff,base);
                }
                else{
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                    fprintf(fp_code,"sll $%d,$%d,2\n",z,y);
                    fprintf(fp_code,"add $%d,$%d,$%s\n",z,z,base);
                    fprintf(fp_code,"lw $%d,%d($%d)\n",z,headoff,z);
                }
                if(z==RD){
                    store(z,opt_code[j].flag_z,RD);
                }
                break;
            }
            case ASNA:{//z[x]=y
                if(opt_code[j].flag_y==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",RT,opt_code[j].y);
                    y = RT;
                }
                else {
                    y = getReg(opt_code[j].y,opt_code[j].flag_y,RT);
                }
                int headoff = arrhead(opt_code[j].z);
                char *base = (opt_code[j].z>0)?"fp":"gp";

                if(opt_code[j].flag_x==F_VAL){
                    fprintf(fp_code,"sw $%d,%d($%s)\n",y,opt_code[j].x*4+headoff,base);
                }
                else {
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);

                    fprintf(fp_code,"sll $%d,$%d,2\n",RS,x);
                    fprintf(fp_code,"add $%d,$%d,$%s\n",RS,RS,base);
                    fprintf(fp_code,"sw $%d,%d($%d)\n",y,headoff,RS);
                }
                break;
            }
            case CALL:{
                save_func_context();
                save_reg_context();
                fprintf(fp_code,"move $fp,$sp\n");
                fprintf(fp_code,"addi $sp,$sp,%d\n",MAX_FUNC_MEM*4);
                fprintf(fp_code,"jal %s\n",tab[btab[opt_code[j].x].head].name);
                fprintf(fp_code,"subi $fp,$fp,%d\n",MAX_FUNC_MEM*4);
                recover_func_context();
                recover_reg_context();
                break;
            }
            case PUSH:{
                if(opt_code[j].flag_x==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",RS,opt_code[j].x);
                    x = RS;
                }
                else {
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,RS);
                }
                fprintf(fp_code,"sw $%d,%d($sp)\n",x,(opt_code[j].z+1)*4);
                break;
            }
            case RET:{
                if(opt_code[j].flag_x==F_VAL){
                    fprintf(fp_code,"li $%d,%d\n",V0,opt_code[j].x);
                }
                else{
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,V0);
                    if(x!=V0){
                        fprintf(fp_code,"move $v0,$%d\n",x);
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
            case BEQ:branch_act_new(j,"beq");break;
            case BNE:branch_act_new(j,"bne");break;
            case BLE:branch_act_new(j,"ble");break;
            case BGE:branch_act_new(j,"bge");break;
            case BGT:branch_act_new(j,"bgt");break;
            case BLT:branch_act_new(j,"blt");break;
            case J:fprintf(fp_code,"j label%d\n",opt_code[j].z);break;
            case SYS_PRINTINT:{
                if(opt_code[j].flag_x==F_VAL) fprintf(fp_code,"li $a0,%d\n",opt_code[j].x);
                else{
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,A0);
                    if(x!=A0){
                        fprintf(fp_code,"move $a0,$%d\n",x);
                    }
                }
                //fprintf(fp_code,"move $v1,$v0\n");
                fprintf(fp_code,"li $v0,%d\n",1);
                fprintf(fp_code,"syscall\n");
                //fprintf(fp_code,"move $v0,$v1\n");
                break;
            }
            case SYS_PRINTCH:{
                if(opt_code[j].flag_x==F_VAL) fprintf(fp_code,"li $a0,%d\n",opt_code[j].x);
                else {
                    x = getReg(opt_code[j].x,opt_code[j].flag_x,A0);
                    if(x!=A0){
                        fprintf(fp_code,"move $a0,$%d\n",x);
                    }
                }
                //fprintf(fp_code,"move $v1,$v0\n");
                fprintf(fp_code,"li $v0,%d\n",11);
                fprintf(fp_code,"syscall\n");
                //fprintf(fp_code,"move $v0,$v1\n");
                break;
            }
            case SYS_PRINTSTR:{
               // if(opt_code[j].x==-1) print_newline();
                //else{
                //fprintf(fp_code,"move $v1,$v0\n");
                fprintf(fp_code,"la $a0,str%d\n",opt_code[j].x);
                fprintf(fp_code,"li $v0,%d\n",4);
                fprintf(fp_code,"syscall\n");
                //fprintf(fp_code,"move $v0,$v1\n");
                //}
                break;
            }
            case SYS_READCH:{
                //fprintf(fp_code,"move $v0,$v1\n");
                fprintf(fp_code,"li $v0,%d\n",12);
                fprintf(fp_code,"syscall\n");
                //fprintf(fp_code,"move $v0,$v1\n");
                break;
            }
            case SYS_READINT:{
                //fprintf(fp_code,"move $v1,$v0\n");
                fprintf(fp_code,"li $v0,%d\n",5);
                fprintf(fp_code,"syscall\n");
                //fprintf(fp_code,"move $v0,$v1\n");
                break;
            }
            }
        //condition branch(not loop) suc write back
        }
        p = ins[i]->next;
        while(p){
            j = p->p->var;
            if(p->p->type==F_VAR){
                if(j>0&&regocc[regs[j-1]]>1||j<0&&regocc[regs[getGLANew(j)-1]]>1){
                       if(debug) printf("exc:%d\n",j);
                    store_wd(j);
                }
            }
            p = p->next;
        }
    }

}
