#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED
#define MAXREG 22
#define MAXTMPSTK 200
#define CMAX 10000
#define LBMAX 10000
#define STRMAX 2000

#define	ADD 	0
#define	SUB	    1
#define	MUL 	2
#define	DIV 	3
#define	GETA	4
#define	ASNA	5
#define	BEQ 	6
#define	BNE 	7
#define	BGE 	8
#define	BGT 	9
#define	BLT 	10
#define	BLE 	11
#define	J 	    12
#define	CALL	13
#define	PUSH	14
#define	RET	    15
#define RET0    23
#define MOVE    16

#define SYS_READCH 17
#define SYS_READINT 18
#define SYS_PRINTSTR 19
#define SYS_PRINTINT 20
#define SYS_PRINTCH 21

#define GLV     0

#define F_VAL   1
#define F_VAR   2
#define F_LBL   3
#define F_PARAM 4
#define F_FUNC  5
#define F_STR   6
#define F_ARR   7

struct order4{
    int f;
    int x,y,z;
    int flag_x,flag_y,flag_z;//flag==1 for var,2 for label(-1 for global tmp_register),flag==0 for value,
};

struct order4 code4[CMAX];
extern int c4p;

char stringlist[STRMAX][100];
extern int strlp ;

extern int tmp_var_cont;
extern int label_cont;
extern int labels[LBMAX];

extern int func_idx;

void tmp_reg_free(int tmp);
void clear_tmp_stack();
int tmp_reg_apply();
int apply_t();
int apply_label();
void fill_Border(int label);
void fill_Border_to(int label,int fcp);

int emit1(int f,int x,int flag_x);
int emit3(int f,int x,int y,int z,int flag_x,int flag_y);
int emit2(int f,int x,int z,int flag_x,int flag_z);
int emit0(int f);
void emitjmain();

void output_code4();

extern struct loopblk* head;

#endif // MEMORY_H_INCLUDED
