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
struct DAG* searchOneGraphZ(struct DAG* root,struct node_list_ele* x,struct node_list_ele* y,int op);
struct node_list_ele* insert_list_node(int isinit,int x,int flag_x);
void searchGraph(struct DAG** list);
int fit2(struct DAG* r);
void searchOneGraph(struct DAG* root,struct DAG** list);

struct node_list_ele* list_head=NULL;
struct DAG *graph_root_head=NULL;


int getBaseBlock(int bidx){//return base blocks in one function
    int i,blkhead=0;
    static int l = 0;
    int blkp = 0;//pointer in blocklist
    for(i = btab[bidx].code_st;i < btab[bidx].code_en;i ++){
        if(blkhead==-1) blkhead=1;
        switch(code4[i].f){
            case BEQ:case BNE:case BGE:case BGT:case BLE:case BLT:case J:blkhead=-1;break;
        }
        while(l<label_cont&&labels[l]==i) {l++;blkhead=1;}
        if(i==btab[bidx].code_st) blkhead = 1;
        if(blkhead!=1) continue;
        bblocklist[blkp].st = i;
        if(blkp>0) bblocklist[blkp-1].en = i-1;
        blkp++;
        blkhead = 0;
    }
    bblocklist[blkp-1].en = i;
    return blkp;
}
void localExpDAG(int bbidx){
    list_head=NULL;
    graph_root_head=NULL;

    int i;
    struct node_list_ele *x,*y,*z;//node index of x,y,z
    for(i = bblocklist[bbidx].st;i < bblocklist[bbidx].en;i ++){
            if(debug) printf("%d\t%d\n",i,code4[i].f);
        switch(code4[i].f){
            case BEQ:case BNE:case BGE:case BGT:case BLE:case BLT:case J:
            case ASNA:case CALL:case SYS_READCH:case SYS_READINT:
            case SYS_PRINTCH:case SYS_PRINTINT:case SYS_PRINTSTR:case RET:case RET0:case PUSH:{
                emitneworder();
                opt_emit(code4[i].f,code4[i].x,code4[i].y,code4[i].z,code4[i].flag_x,code4[i].flag_y,code4[i].flag_z);
                break;
            }
            case MOVE:
                search_opx(&x,&z,&code4[i]);
                (z)->node = (x)->node;
                break;
            case ADD:case SUB:case MUL:case DIV:{
                search_xopy(&x,&y,&z,&code4[i]);
                if(debug){
                    printf("ADD:%d %d %d\n",x->va,y->va,z->va);
                }
                addtoDAG(x,y,z,code4[i].f);
                break;
            }
        }
    }

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
    struct DAG* r=graph_root_head,*p;
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
        while(r&&r->next!=NULL) r = r->next;
        if(r==NULL){
            r = p;
            graph_root_head = p;
        }
        else
            r->next = p;
        p->next = NULL;
    }
    p->left = x->node;
    p->right = y->node;
    x->node->parents[x->node->parpt ++] = p;
    y->node->parents[y->node->parpt ++] = p;
    z->node = p;
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
    return p;
}
struct DAG* findZnode(struct node_list_ele* x,struct node_list_ele* y,int op){
    struct DAG* p,*q;
    for(p = graph_root_head;p!=NULL;p = p->next){
        q = searchOneGraphZ(p,x,y,op);
        if(q) return q;
    }
    return NULL;
}
struct DAG* searchOneGraphZ(struct DAG* root,struct node_list_ele* x,struct node_list_ele* y,int op){
    struct DAG*p = x->node,*q = y->node;
    if(root->left!=NULL&&root->right!=NULL){
        if(root->left==p&&root->right==q&&root->content==op){
            return root;
        }
        p = searchOneGraphZ(root->left,x,y,op);
        q = searchOneGraphZ(root->right,x,y,op);
        if(p!=NULL){
            return p;
        }
        if(q!=NULL){
            return q;
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
    searchGraph(list);
    struct DAG* r;
    struct DAG* queue = NULL,*tail;
    int i;
    int x,y,fx,fy;
    while(cont<DAG_node_cont){
        if(!cont){
            queue = graph_root_head;
            queue->qlast = NULL;
            tail = graph_root_head;
            graph_root_head->inqueue = 1;
            r = graph_root_head->left;
            while(r!=NULL&&fit2(r)){
                tail->qnext = r;
                r->qlast = tail;
                tail = r;
                r = r->left;
                cont ++;
            }
        }
        else{

            if(debug) {
                printf("cont = %d\n",cont);
            }
            for(i = 0;i < DAG_node_cont;i ++){
                if(fit2(list[i])){
                    break;
                }
            }
            if(debug&&cont==4) {
                printf("i = %d DAG=%d\n",i,DAG_node_cont-);
            }
            if(i!=DAG_node_cont){
                tail->qnext = list[i];
                list[i]->qlast = tail;
                tail = list[i];
                tail->qnext = NULL;
                list[i]->inqueue = 1;
                r = tail->left;
                while(r!=NULL&&fit2(r)){
                    tail->qnext = r;
                    r->qlast = tail;
                    tail = r;
                    r = r->left;
                    cont ++;
                }
            }
        }
    }
    for(r = tail;r !=NULL;r = r->qlast){
        if(r->left==NULL)
            continue;
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
    list_head->next=NULL;
}
int fit2(struct DAG* r){
    int i,pt;
    pt = r->parpt;
    if(r->inqueue)
        return 0;
    for(i = 0;i < pt;i ++){
        if(r->parents[i]->inqueue==0)
            break;
    }
    if(i!=pt)
        return 0;
    if(pt==0) return 1;
    return 1;
}
void searchGraph(struct DAG** list){
    struct DAG* p;
    for(p = graph_root_head;p!=NULL;p = p->next){
        searchOneGraph(p,list);
    }
}
void searchOneGraph(struct DAG* root,struct DAG** list){
    if(list[root->index]!=root){
        list[root->index] = root;
    }
    if(root->left!=NULL&&root->right!=NULL){
        searchOneGraph(root->left,list);
        searchOneGraph(root->right,list);
    }
}

void dismLocExp(){
    int i,j;
    int blks;
    for(i = 1;i <= bp;i ++){
        blks=getBaseBlock(i);
        for(j = 0;j < blks;j ++){
            DAG_node_cont = 0;
            localExpDAG(j);
            /*output_opt_code4();*/
        }
    }
}
