#include<stdio.h>
#include<stdlib.h>
#include"table.h"
#include"optimize.h"
void emitneworder();
void localExpDAG(int bbidx);
void search_xopy(struct node_list_ele** xx,struct node_list_ele** yy,struct node_list_ele** zz,struct order4* code);
void search_opx(struct node_list_ele** xx,struct node_list_ele** zz,struct order4* code);
void addtoDAG(struct node_list_ele *x,struct node_list_ele *y,struct node_list_ele *z,int op);
struct DAG* new_DAG_node();
struct DAG* findZnode(struct node_list_ele* x,struct node_list_ele* y,int op);
struct node_list_ele* insert_list_node(int isinit,int x,int flag_x);
int fit2(struct DAG* r);
void addtoDAGxz(struct node_list_ele* x,struct node_list_ele* z);

struct node_list_ele* list_head=NULL;
struct DAG *graph_root_head=NULL,*graph_tail;


int getBaseBlock(int bidx){//return base blocks in one function
    int i,blkhead=0;
    static int l = 0;
    int blkp = 0;//pointer in blocklist
    for(i = btab[bidx].code_st;i < btab[bidx].code_en;i ++){
        if(blkhead==-1) blkhead=1;
        while(l<label_cont&&labels[l]==i) {l++;blkhead=1;}
        if(i==btab[bidx].code_st) blkhead = 1;
        if(blkhead==1){
            bblocklist[blkp].st = i;
            if(blkp>0) bblocklist[blkp-1].en = i-1;
            blkp++;
            blkhead = 0;
        }
        switch(code4[i].f){
            case BEQ:case BNE:case BGE:case BGT:case BLE:case BLT:case J:blkhead=-1;break;
        }
    }
    bblocklist[blkp-1].en = i-1;
    return blkp;
}
void localExpDAG(int bbidx){
    list_head = NULL;
    graph_root_head = NULL;
    graph_tail = graph_root_head;
    DAG_node_cont = 0;
    int i;
    struct node_list_ele *x,*y,*z;//node index of x,y,z
    fill_label(bblocklist[bbidx].st);
    for(i = bblocklist[bbidx].st;i <= bblocklist[bbidx].en;i ++){
        switch(code4[i].f){
            case BEQ:case BNE:case BGE:case BGT:case BLE:case BLT:case J:
            case GETA:case ASNA:case CALL:case SYS_READCH:case SYS_READINT:
            case SYS_PRINTCH:case SYS_PRINTINT:case SYS_PRINTSTR:case RET:case RET0:{
                emitneworder();
                printf("2333\n");
                opt_emit(code4[i].f,code4[i].x,code4[i].y,code4[i].z,code4[i].flag_x,code4[i].flag_y,code4[i].flag_z);
                break;
            }
            case MOVE:
                search_opx(&x,&z,&code4[i]);
                addtoDAGxz(x,z);
                break;
            case ADD:case SUB:case MUL:case DIV:{
                search_xopy(&x,&y,&z,&code4[i]);
                addtoDAG(x,y,z,code4[i].f);
                break;
            }
        }
    }
    fill_label(bblocklist[bbidx].en);

}
//xx = &x
void search_xopy(struct node_list_ele** xx,struct node_list_ele** yy,struct node_list_ele** zz,struct order4* code){
    struct node_list_ele* pl;
    *xx = NULL;*yy = NULL;*zz = NULL;
    for(pl = list_head;pl!=NULL;pl=pl->next){
        if(code->flag_x==pl->type&&code->x==pl->va&&*xx==NULL){
            *xx = pl;
        }
        if(code->flag_y==pl->type&&code->y==pl->va&&*yy==NULL){
            *yy = pl;
        }
        if(code->flag_z==pl->type&&code->z==pl->va&&*zz==NULL){
            if(!(pl->isinit))
                *zz = pl;
        }
        if(pl->next==NULL)
            break;
    }
    if(*xx==NULL){
        *xx=insert_list_node(1,code->x,code->flag_x);
    }
    if(*yy==NULL){
        *yy = insert_list_node(1,code->y,code->flag_y);
    }
    if(*zz==NULL){
        *zz = insert_list_node(0,code->z,code->flag_z);
    }
}
void search_opx(struct node_list_ele** xx,struct node_list_ele** zz,struct order4* code){
    struct node_list_ele* pl;
    *xx = NULL;*zz = NULL;
    for(pl = list_head;pl!=NULL;pl=pl->next){
        if(code->flag_x==pl->type&&code->x==pl->va){
            *xx = pl;
        }
        if(code->flag_z==pl->type&&code->z==pl->va&&*zz==NULL){
            if(!(pl->isinit))
                *zz = pl;
        }
        if(pl->next==NULL)
            break;
    }
    if(*xx==NULL){
        *xx = insert_list_node(1,code->x,code->flag_x);
    }
    if(*zz==NULL){
        *zz = insert_list_node(0,code->z,code->flag_z);
        //insert backwards to get newest node
    }
}
void addtoDAG(struct node_list_ele *x,struct node_list_ele *y,struct node_list_ele *z,int op){
    struct DAG *p;
    if(x->node==NULL){
        x->node = new_DAG_node();
        x->node->leaf_node = x;
    }
    if(y->node==NULL){
        y->node = new_DAG_node();
        y->node->leaf_node = y;
    }
    p = findZnode(x,y,op);
    if(p==NULL){
        p = new_DAG_node();
        p->content = op;
    }
    p->left = x->node;
    p->right = y->node;
    x->node->parents[x->node->parpt ++] = p;
    y->node->parents[y->node->parpt ++] = p;
    z->node = p;
}
void addtoDAGxz(struct node_list_ele* x,struct node_list_ele* z){
    if(x->node==NULL){
        x->node = new_DAG_node();
        x->node->leaf_node = x;
    }
    z->node = x->node;
}
struct DAG* new_DAG_node(){
    struct DAG* p = (struct DAG*)malloc(sizeof(struct DAG));
    p->content = 0;
    p->index = DAG_node_cont ++;
    p->parpt = 0;
    p->left = NULL;
    p->right = NULL;
    p->next = NULL;
    p->inqueue = 0;
    p->qnext = NULL;
    p->qlast = NULL;
    p->leaf_node = NULL;
    if(graph_tail){
        graph_tail->next = p;
    }
    else{
        graph_root_head = p;
    }
    graph_tail = p;
    return p;
}
struct DAG* findZnode(struct node_list_ele* x,struct node_list_ele* y,int op){
    struct DAG* p;
    for(p = graph_root_head;p!=NULL;p = p->next){
        if(p->left==x->node&&p->right==y->node&&p->content==op){
            return p;
        }
    }
    return NULL;
}
struct node_list_ele* insert_list_node(int isinit,int x,int flag_x){
    struct node_list_ele* p;
    p = (struct node_list_ele*)malloc(sizeof(struct node_list_ele));
    p->va = x;
    p->type = flag_x;
    p->isinit = isinit;
    p->node = NULL;
    p->next = list_head;
    list_head = p;
    return p;
}
void emitneworder(){
    int cont = 0;
    struct DAG** list = (struct DAG**)malloc(sizeof(struct DAG*)*DAG_node_cont);
    struct DAG* r;
    struct node_list_ele* p;
    struct DAG *qtail=NULL;
    int i;
    int x,y,fx,fy;
    for(r = graph_root_head;r!=NULL;r = r->next){
        list[r->index] = r;
    }

    for(p = list_head;p ;p = p->next){
        cont++;
    }
    cont = 0;
    while(cont<DAG_node_cont){
        for(i = 0;i < DAG_node_cont;i ++){
            if(fit2(list[i])){
                break;
            }
        }
        if(i!=DAG_node_cont){
            if(qtail!=NULL){
                qtail->qnext = list[i];
            }
            list[i]->qlast = qtail;
            qtail = list[i];
            qtail->qnext = NULL;
            list[i]->inqueue = 1;
            cont ++;
            r = qtail->left;
            while(r!=NULL&&fit2(r)){
                r->inqueue = 1;
                qtail->qnext = r;
                r->qlast = qtail;
                qtail = r;
                r = r->left;
                cont ++;
            }
        }
    }
    for(r = qtail;r !=NULL;r = r->qlast){
        if(r->left==NULL){
            for(p = list_head;p!=NULL;p = p->next){
                if(p->node==r&&p->type!=F_VAL&&p->isinit==0){
                    opt_emit(MOVE,r->leaf_node->va,0,p->va,r->leaf_node->type,0,p->type);
                }
            }
            continue;
        }
        if(r->left->left==NULL){
            x = r->left->leaf_node->va;
            fx = r->left->leaf_node->type;
        }
        else{
            x = r->left->index;
            fx = F_TMPV;
        }
        if(r->right->left==NULL){
            y = r->right->leaf_node->va;
            fy = r->right->leaf_node->type;
        }
        else{
            y = r->right->index;
            fy = F_TMPV;
        }
        opt_emit(r->content,x,y,r->index,fx,fy,F_TMPV);
    }

    /*
    */
    struct node_list_ele *tmp ;
    p=list_head;
    while(p!=NULL){
        tmp = p;
        p = p->next;
        free(tmp);
    }
    for(i = 0;i < DAG_node_cont;i ++){
        free(list[i]);
    }
    list_head=NULL;
    graph_root_head = NULL;
    graph_tail = NULL;
    DAG_node_cont = 0;
}
int fit2(struct DAG* r){
    int i,pt;
    pt = r->parpt;
    if(r->inqueue)
        return 0;
    if(pt==0) return 1;
    for(i = 0;i < pt;i ++){
        if(r->parents[i]->inqueue==0)
            break;
    }
    if(i!=pt)
        return 0;
    return 1;
}

void dismLocExp(){
    int i,j;
    int blks;
    for(i = 1;i <= bp;i ++){
        blks=getBaseBlock(i);
        for(j = 0;j < blks;j ++){
            localExpDAG(j);
        }

    }
    //call main
    opt_emitjmain();
    output_opt_code4();
}

struct uvd *out,*in;
void active(){

}
