#include<stdio.h>
#include<stdlib.h>
#include"table.h"
#include"optimize.h"

struct basic_blk bblocklist[MAXBBLK];
int bsc_blks;
struct node_list_ele* list_head=NULL,*list_tail;
struct DAG *graph_root_head=NULL,*graph_tail;

struct udv **use,**def;
struct udv_ **outs,**ins;

int conflict[MAXV][MAXV];
int conf_len;
struct rename_tab *glb;
int references[MAXV] = {0};
int regs[MAXV]={0};//-1 for no;0 for not yet;>0 distributed
int regocc[MAXSREG];

int tmp_regs[MAXTREG];//each reg
int tmp_reg_pt;
int tmp_stack[MAXTMPSTK];
int tmp_stk_pt;

void search_xopy(struct node_list_ele** xx,struct node_list_ele** yy,struct node_list_ele** zz,struct order4* code);
void search_opx(struct node_list_ele** xx,struct node_list_ele** zz,struct order4* code);
void addtoDAG(struct node_list_ele *x,struct node_list_ele *y,struct node_list_ele *z,int op);
struct DAG* new_DAG_node();
struct DAG* findZnode(struct node_list_ele* x,struct node_list_ele* y,int op);
struct node_list_ele* insert_list_node(int isinit,int x,int flag_x);
int fit2(struct DAG* r);
void addtoDAGxz(struct node_list_ele* x,struct node_list_ele* z);
void active(int bidx,int pass);

int getBaseBlock(int bidx){//return base blocks in one function
    int i,j,blkhead=0;
    static int l = 0;
    int blkp = 0;//pointer in blocklist

    for(i = btab[bidx].code_st;i < btab[bidx].code_en;i ++){
        if(blkhead==-1) blkhead=1;
        if(l<label_cont&&labels[l]==i) {
            for(j = 0;j < blkp;j ++){
                int st = bblocklist[j].st;
                int en = bblocklist[j].en;
                if(code4[st].flag_z==F_LBL&&code4[st].z==l){
                    bblocklist[j].sucs[bblocklist[j].sucpt++] = blkp-1;
                }
                if(code4[en].flag_z==F_LBL&&code4[en].z==l){
                    bblocklist[j].sucs[bblocklist[j].sucpt++] = blkp-1;
                }
            }
            l++;
            blkhead=1;
        }
        if(i==btab[bidx].code_st) blkhead = 1;
        if(blkhead==1){
            bblocklist[blkp].st = i;
            bblocklist[blkp].sucs[0] = blkp+1;
            bblocklist[blkp].sucpt = 1;
            if(blkp>0) bblocklist[blkp-1].en = i-1;
            blkp++;
            blkhead = 0;
        }
        switch(code4[i].f){
            case BEQ:case BNE:case BGE:case BGT:case BLE:case BLT:case J:{
                int labelline = (code4[i].flag_z==F_LBL)?labels[code4[i].z]:-1 ;
                for(j = 0;j < blkp;j ++){
                    if(bblocklist[j].st<=labelline&&bblocklist[j].en>=labelline){
                        bblocklist[blkp-1].sucs[bblocklist[blkp-1].sucpt++] = j;
                        break;
                    }
                }
                blkhead=-1;
                break;
            }
            case GETA:case ASNA:case CALL:case SYS_READCH:case SYS_READINT:
            case SYS_PRINTCH:case SYS_PRINTINT:case SYS_PRINTSTR:case RET:case RET0:case PUSH:
                blkhead=-1;
                break;
        }
    }
    bblocklist[blkp-1].en = i-1;
    bblocklist[blkp-1].sucpt = 0;
    return blkp;
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
        if(code->flag_x==pl->type&&code->x==pl->va&&*xx==NULL){
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
    }
    if(y->node==NULL){
        y->node = new_DAG_node();
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
    p->varnodes = NULL;
    p->vn_tail = NULL;
    if(graph_tail){
        graph_tail->next = p;
    }
    else{
        graph_root_head = p;
    }
    graph_tail = p;
    return p;
}
void assigntoDAG(struct node_list_ele* x){
    struct var_node_for_DAG* p = (struct var_node_for_DAG*)malloc(sizeof(struct var_node_for_DAG));
    p->ele = x;
    p->next = NULL;
    if(x->node->varnodes==NULL){
        x->node->varnodes = p;
    }
    else
        x->node->vn_tail->next = p;
    x->node->vn_tail = p;
}
void assignHead(struct node_list_ele* x){
    struct var_node_for_DAG* p = (struct var_node_for_DAG*)malloc(sizeof(struct var_node_for_DAG));
    p->ele = x;
    p->next = x->node->varnodes;
    x->node->varnodes = p;
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
    p->last = NULL;
    if(list_head!=NULL){
        list_head->last = p;
    }
    if(list_head==NULL) list_tail = p;
    list_head = p;
    return p;
}

struct node_list_ele* add_list_node(int isinit,int x,int flag_x){
    struct node_list_ele* p;
    //list_tail != NULL
    p = (struct node_list_ele*)malloc(sizeof(struct node_list_ele));
    p->va = x;
    p->type = flag_x;
    p->isinit = isinit;
    p->node = NULL;
    p->next = NULL;
    if(list_tail!=NULL) list_tail->next = p;
    p->last = list_tail;
    list_tail = p;
    return p;
}

void emitneworder(int bbidx,int btidx){
    int cont = 0;
    struct DAG** list = (struct DAG**)malloc(sizeof(struct DAG*)*DAG_node_cont);
    struct DAG* r;
    struct node_list_ele* p;
    struct DAG *qtail=NULL;
    struct udv_* q;
    int i;
    int x,y,fx ,fy,z,fz;
    for(r = graph_root_head;r!=NULL;r = r->next){
        list[r->index] = r;
    }
    for(p = list_head;p ;p = p->next){
        p->isact = 0;
    }
    for(p = list_tail;p ;p = p->last){
        if(p->isinit==1){
            assigntoDAG(p);
        }
    }
    for(p = list_tail;p ;p = p->last){
        if(p->isinit==0&&p->type==F_VAR&&p->va<0){
            assigntoDAG(p);
            p->isact = 1;
            continue;
        }
        q = outs[bbidx]->next;
        while(q){
            if(q->p->type==p->type&&q->p->var==p->va){
                assigntoDAG(p);
                p->isact = 1;
                break;
            }
            q = q->next;
        }
    }
    //if(debug) printf("act ok\n");
    //add into queue
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
    //connect last line to DAG
    int nextcode = bblocklist[bbidx].en;
    if(nextcode<btab[btidx].code_en)
    for(p = list_head;p!=NULL;p = p->next){
        if(p->isact==0){
            switch(code4[nextcode].f){
                case SYS_READCH:case SYS_READINT:case SYS_PRINTCH:case SYS_PRINTINT:case RET://GLV
                    if(p->type==F_VAR&&p->va==GLV){
                        if(p->node->varnodes!=NULL){
                            code4[nextcode].flag_x = p->node->varnodes->ele->type;
                            code4[nextcode].x = p->node->varnodes->ele->va;
                        }
                        else{
                            assigntoDAG(p);
                            p->isact = 1;
                        }
                    }
                    break;
                case BEQ:case BNE:case BGE:case BGT:case BLE:case BLT:case ASNA:case PUSH://x,y
                case GETA://y,z
                    if(code4[nextcode].flag_x==p->type&&code4[nextcode].x==p->va){
                        if(p->node->varnodes!=NULL){
                            code4[nextcode].flag_x = p->node->varnodes->ele->type;
                            code4[nextcode].x = p->node->varnodes->ele->va;
                        }
                        else{
                            assigntoDAG(p);
                            p->isact = 1;
                        }
                    }
                    if(code4[nextcode].flag_y==p->type&&code4[nextcode].y==p->va){
                        if(p->node->varnodes!=NULL){
                            code4[nextcode].flag_y = p->node->varnodes->ele->type;
                            code4[nextcode].y = p->node->varnodes->ele->va;
                        }
                        else{
                            assigntoDAG(p);
                            p->isact = 1;
                        }
                    }
                    if(code4[nextcode].flag_z==p->type&&code4[nextcode].z==p->va){
                        if(p->node->varnodes!=NULL){
                            code4[nextcode].flag_z = p->node->varnodes->ele->type;
                            code4[nextcode].z = p->node->varnodes->ele->va;
                        }
                        else{
                            assigntoDAG(p);
                            p->isact = 1;
                        }
                    }
            }
        }
    }
    //pruning
    cont = 0;
    do{
       if(cont>0){
            int flag = 0;
            if(r->qlast!=NULL){
                r->qlast->qnext = r->qnext;
            }
            if(r->qnext!=NULL){
                r->qnext->qlast = r->qlast;
            }
            else
                qtail = r->qlast;
            if(r->left!=NULL){
                for(i = 0;i < r->left->parpt-1;i ++){
                    if(r->left->parents[i]==r){
                        flag = 1;
                    }
                    if(flag){
                        r->left->parents[i] = r->left->parents[i+1];
                    }
                }
                r->left->parpt--;
            }
            if(r->right!=NULL){
                for(i = 0;i < r->right->parpt-1;i ++){
                    if(r->right->parents[i]==r){
                        flag = 1;
                    }
                    if(flag){
                        r->right->parents[i] = r->right->parents[i+1];
                    }
                }
                r->right->parpt--;
            }
            free(r);
            cont = 0;
       }
       for(r = qtail;r !=NULL;r = r->qlast){
            if(r->parpt==0&&r->varnodes==NULL){
                cont = 1;
                break;
            }
       }
    }while(cont>0);
    //output nodes
    for(r = qtail;r !=NULL;r = r->qlast){
        if(r->left==NULL){//left=NULL=>right=NULL
            continue;
        }
        if(r->varnodes==NULL&&r->parpt==0)//not active final results are killed
            continue;
        if(r->left->varnodes!=NULL){
            p = r->left->varnodes->ele;
            x = p->va;
            fx = p->type;
        }
        else{
            x = r->left->index;
            fx = F_TMPV;
        }
        if(r->right->varnodes!=NULL){
            p = r->right->varnodes->ele;
            y = p->va;
            fy = p->type;
        }
        else{
            y = r->right->index;
            fy = F_TMPV;
        }
        if(fx==F_VAL&&fy==F_VAL){
            //list_tail!=NULL
            fz = F_VAL;
            switch(r->content){
                case ADD:z = x+y;break;
                case SUB:z = x-y;break;
                case MUL:z = x*y;break;
                case DIV:z = x/y;break;
                default:z = 0;fz = 0;
            }
            if(fz==F_VAL){
                add_list_node(1,z,fz);
                list_tail->node = r;
                assignHead(list_tail);
            }
            continue;
        }
        if(r->varnodes!=NULL){
            p = r->varnodes->ele;
            z = p->va;
            fz = p->type;
        }
        else{
            z = r->index;
            fz = F_TMPV;
        }
        opt_emit(r->content,x,y,z,fx,fy,fz);
    }
    for(p = list_head;p!=NULL;p = p->next){
        if(p->type!=F_VAL&&p->isinit==0&&p->isact&&p->node->varnodes!=NULL){
            struct node_list_ele* actvarhead = p->node->varnodes->ele;
            if(p->type!=actvarhead->type||p->va!=actvarhead->va){
                opt_emit(MOVE,actvarhead->va,0,p->va,actvarhead->type,0,p->type);
            }
        }
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
    free(list);
    list_head=NULL;
    list_tail=NULL;
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

void localExpDAG(int bbidx,int btidx){
    list_head = NULL;
    graph_root_head = NULL;
    graph_tail = graph_root_head;
    DAG_node_cont = 0;
    int i;
    struct node_list_ele *x,*y,*z;//node index of x,y,z
    fill_label(bblocklist[bbidx].st);
    for(i = bblocklist[bbidx].st;i <= bblocklist[bbidx].en;i ++){
        switch(code4[i].f){
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
    emitneworder(bbidx,btidx);
    i = bblocklist[bbidx].en;
    opt_emit(code4[i].f,code4[i].x,code4[i].y,code4[i].z,code4[i].flag_x,code4[i].flag_y,code4[i].flag_z);
    fill_label(bblocklist[bbidx].en);

}

int in_set(struct udv *usehead,struct udv *defhead,int x,int flag_x){
    struct udv *p = usehead;
    while(p){
        if(p->var==x&&p->type==flag_x){
            return 1;
        }
        p = p->next;
    }
    p = defhead;
    while(p){
        if(p->var==x&&p->type==flag_x){
            return 1;
        }
        p = p->next;
    }
    return 0;
}
struct udv* new_udv_node(int x,int flag_x){
    struct udv* p= (struct udv*)malloc(sizeof(struct udv));
    p->type = flag_x;
    p->var = x;
    p->next = NULL;
    return p;
}
void compute_in(struct udv *def,struct udv *use,int i){
    //useU(out-def)
    struct udv_ *q = outs[i],*r;
    struct udv *p = def;

    if(outs[i]->next==NULL){
        return;
    }
    else{//out-def
        while(p){
            q = outs[i];
            while(q->next){
                if(q->next->p->type==p->type&&q->next->p->var==p->var){
                    r = q->next;
                    q->next = q->next->next;
                    free(r);
                    break;
                }
                else
                    q = q->next;
            }
            p = p->next;
        }
    }
    q = outs[i]->next;
    while(q){
        r = ins[i];
        while(r->next){
            if(r->next->p->type==q->p->type&&r->next->p->var==q->p->var)
                break;
            r = r->next;
        }
        if(r->next==NULL){
            r->next = q;
            q = q->next;
            r->next->next = NULL;
        }
        else{
            q = q->next;
        }
    }
    outs[i]->next = NULL;
}
void compute_out(int idx){
    int j;
    struct udv_* p,*q;
    for(j = 0;j < bblocklist[idx].sucpt;j ++){
        if(bblocklist[idx].sucs[j]==bsc_blks)
            continue;
        q = ins[bblocklist[idx].sucs[j]]->next;
        while(q){
            p = outs[idx];
            while(p->next){
                if(p->next->p->type==q->p->type&&p->next->p->var==q->p->var)
                    break;
                p = p->next;
            }
            if(p->next==NULL){
                p->next = (struct udv_*)malloc(sizeof(struct udv_));
                p->next->p = q->p;
                p->next->next = NULL;
            }
            q = q->next;
        }
    }
}
void active(int bidx,int pass){
    struct udv **use_tail,**def_tail;
    //int usept = 0,defpt = 0;
    int last_in_len,cur_in_len = 0;
    use = (struct udv**)malloc(sizeof(struct udv*)*bsc_blks);
    use_tail = (struct udv**)malloc(sizeof(struct udv*)*bsc_blks);
    def = (struct udv**)malloc(sizeof(struct udv*)*bsc_blks);
    def_tail = (struct udv**)malloc(sizeof(struct udv*)*bsc_blks);

    struct order4 *codelist = (pass==1)?code4:opt_code;
    int cst,cen;

    int i,j;
    struct udv* p;
    struct udv_* q;
    for(i = 0;i < bsc_blks ;i ++){
        use[i] = NULL;
        def[i] = NULL;
        use_tail[i] = NULL;
        def_tail[i] = NULL;
        if(pass==1){
            cst = bblocklist[i].st;
            cen = bblocklist[i].en;
        }
        else if(pass==2){
            cst = bblocklist[i].opt_st;
            cen = bblocklist[i].opt_en;
        }
        for(j = cst;j <= cen;j++){
                if((codelist[j].flag_x==F_VAR||codelist[j].flag_x==F_PARAM)&&!in_set(use[i],def[i],codelist[j].x,codelist[j].flag_x)){
                    p = new_udv_node(codelist[j].x,codelist[j].flag_x);
                    if(use[i]==NULL){
                        use[i] = p;
                    }
                    else{
                        use_tail[i]->next = p;
                    }
                    use_tail[i] = p;
                }
                if((codelist[j].flag_y==F_VAR||codelist[j].flag_y==F_PARAM)&&!in_set(use[i],def[i],codelist[j].y,codelist[j].flag_y)){
                    p = new_udv_node(codelist[j].y,codelist[j].flag_y);
                    if(use[i]==NULL){
                        use[i] = p;
                    }
                    else{
                        use_tail[i]->next = p;
                    }
                    use_tail[i] = p;
                }
                if((codelist[j].flag_z==F_VAR||codelist[j].flag_z==F_PARAM)&&!in_set(use[i],def[i],codelist[j].z,codelist[j].flag_z)){
                    p = new_udv_node(codelist[j].z,codelist[j].flag_z);
                    if(def[i]==NULL){
                        def[i] = p;
                    }
                    else{
                        def_tail[i]->next = p;
                    }
                    def_tail[i] = p;
                }
        }
    }
    ins = (struct udv_**)malloc(sizeof(struct udv_*)*bsc_blks);
    outs = (struct udv_**)malloc(sizeof(struct udv_*)*bsc_blks);
    for(i = 0;i < bsc_blks;i ++){
        ins[i] = (struct udv_*)malloc(sizeof(struct udv_));
        ins[i]->next = NULL;
        outs[i] = (struct udv_*)malloc(sizeof(struct udv_));
        outs[i]->next = NULL;
        p = use[i];
        q = ins[i];
        while(p){
            q->next = (struct udv_*)malloc(sizeof(struct udv_));
            q = q->next;
            q->p = p;
            q->next = NULL;
            p = p->next;
        }
    }

    if(debug) printf("in out init ok\n");
    do{
        last_in_len = cur_in_len;
        cur_in_len = 0;
        for(i = bsc_blks-1;i >= 0 ;i --){
            compute_out(i);
            //if(debug) printf("out ok\n");
            compute_in(def[i],use[i] ,i);
            //if(debug) printf("in ok\n");
            q = ins[i]->next;
            while(q){
                cur_in_len ++;
                q = q->next;
            }
        }

    if(debug) printf("ins=%d\t%d\n",last_in_len,cur_in_len);
    }while(last_in_len!=cur_in_len);
    for(i = bsc_blks-1;i >= 0 ;i --){
        compute_out(i);
    }
}

void renameActVar(int btidx){
    int i,j;
    struct rename_tab *head = NULL,*tail;
    int cont = 1;
    struct rename_tab *p;
    struct udv_ *q;
    int x,y,z;

    for(i = 0;i < bsc_blks;i ++){
        q = ins[i]->next;
        while(q){
            if(q->p->type!=F_VAR||q->p->var<=0){
                q = q->next;
                continue;
            }
            p = head;
            while(p){
                if(q->p->type==F_VAR&&p->o==q->p->var){
                    break;
                }
                p = p->next;
            }
            if(p==NULL){
                p = (struct rename_tab *)malloc(sizeof(struct rename_tab));
                p->o = q->p->var;
                p->n = cont++;
                p->next = NULL;
                if(head==NULL){
                    head = p;
                }
                else
                    tail->next = p;
                tail = p;
            }
            q = q->next;
        }
    }
    for(i = 0;i < bsc_blks;i ++){
        for(j = bblocklist[i].opt_st;j <= bblocklist[i].opt_en;j ++){
            x = 0;y = 0;z = 0;
            p = head;
            while(p){
                if(opt_code[j].flag_x==F_VAR){
                    if(p->o==opt_code[j].x&&!x){
                        opt_code[j].x = p->n;
                        x = 1;
                    }
                }
                else
                        x = 1;
                if(opt_code[j].flag_y==F_VAR){
                    if(p->o==opt_code[j].y&&!y){
                        opt_code[j].y = p->n;
                        y = 1;
                    }
                }
                else
                        y = 1;
                if(opt_code[j].flag_z==F_VAR){
                    if(p->o==opt_code[j].z&&!z){
                        opt_code[j].z = p->n;
                        z = 1;
                    }
                }
                else
                        z = 1;
                if(x&&y&&z)
                    break;
                p = p->next;
            }
        }
    }
    p = head;
    while(p){
        tail = p;
        p = p->next;
        free(tail);
    }
    btab[btidx].vd = cont;
}

void freeActMem(){
    int i;
    struct udv *p;
    void *tmp;
    struct udv_ *q;
    for(i = 0;i < bsc_blks;i ++){
        p = use[i];
        while(p){
            tmp = p;
            p = p->next;
            free(tmp);
        }
        p = def[i];
        while(p){
            tmp = p;
            p = p->next;
            free(tmp);
        }
        q = ins[i];
        while(q){
            tmp = q;
            q = q->next;
            free(tmp);
        }
        q = outs[i];
        while(q){
            tmp = q;
            q = q->next;
            free(tmp);
        }
    }
    free(ins);
    free(outs);
}

int getGlActV(int vd,struct rename_tab *head){
    struct rename_tab *tail;
    head = NULL;

    int cont = vd;
    struct rename_tab *p;
    struct udv_ *q;
    int i;

    for(i = 0;i < bsc_blks;i ++){
        q = ins[i]->next;
        while(q){
            if(q->p->type!=F_VAR||q->p->var>=0){
                q = q->next;
                continue;
            }
            p = head;
            while(p){
                if(q->p->type==F_VAR&&p->o==q->p->var){
                    break;
                }
                p = p->next;
            }
            if(p==NULL){
                p = (struct rename_tab *)malloc(sizeof(struct rename_tab));
                p->o = q->p->var;
                p->n = cont++;
                p->next = NULL;
                if(head==NULL){
                    head = p;
                }
                else
                    tail->next = p;
                tail = p;
            }
            q = q->next;
        }
    }
    return cont;
}
int getGLANew(int mcount){
    struct rename_tab *g;
    g = glb;
    while(g){
        if(g->o==mcount){
            return g->n;
        }
    }
    return -1;
}
int getGLAOrg(int mcount){
    struct rename_tab *g;
    g = glb;
    while(g){
        if(g->n==mcount){
            return g->o;
        }
    }
    return 0;
}
void freeGlb(){
    struct rename_tab *g,*t;
    g = glb;
    while(g){
        t = g;
        g = g->next;
        free(t);
    }
}
void createConfGraph(int btidx){
    active(btidx,2);

    int i,j;
    struct udv_ *p,*q;
    int idx1,idx2;

    int allvd = getGlActV(btab[btidx].vd,glb);
    conf_len = allvd-1;

    for(i = 0;i < conf_len;i ++){
        for(j = 0;j < conf_len;j ++){
            conflict[i][j] = 0;
        }
    }

    for(i = 0;i < bsc_blks;i ++){
        p = ins[i]->next;
        while(p){
            if(p->p->type!=F_VAR||p->p->var==0){
                p = p->next;
                continue;
            }
            q = p->next;
            while(q){
                if(q->p->type!=F_VAR||q->p->var==0){
                    q = q->next;
                    continue;
                }
                if(p->p->var>0)
                    idx1 = p->p->var-1;
                else if(p->p->var<0){
                    idx1 = getGLANew(p->p->var)-1;
                }
                if(q->p->var>0)
                    idx2 = q->p->var-1;
                else if(q->p->var<0){
                    idx2 = getGLANew(q->p->var)-1;
                }
                if(!conflict[idx1][idx2]) conflict[idx1][idx2] = 1;
                if(!conflict[idx2][idx1]) conflict[idx2][idx1] = 1;
                q = q->next;
            }
            p = p->next;
        }
    }

}
int conf_sum(int idx){
    int cont = 0;
    int i;
    for(i = 0;i < conf_len;i ++){
        cont += conflict[idx][i];
    }
    return cont;
}

void count_refs(){
    int i,idx;
    for(i = 0;i < conf_len;i ++){
        references[i] = 0;
    }
    for(i = bblocklist[0].opt_st;i <= bblocklist[bsc_blks-1].opt_en;i ++){
        if(opt_code[i].flag_x==F_VAR){
            if(opt_code[i].x<0){
                idx = getGLANew(opt_code[i].x)-1;
            }
            else if(opt_code[i].x>0){
                idx = opt_code[i].x-1;
            }
            if(opt_code[i].x) references[idx] ++;
        }
        if(opt_code[i].flag_y==F_VAR){
            if(opt_code[i].y<0){
                idx = getGLANew(opt_code[i].y)-1;
            }
            else if(opt_code[i].x>0){
                idx = opt_code[i].x-1;
            }
            if(opt_code[i].y) references[idx] ++;
        }
        if(opt_code[i].flag_z==F_VAR){
            if(opt_code[i].z<0){
                idx = getGLANew(opt_code[i].z);
            }
            else if(opt_code[i].z>0){
                idx = opt_code[i].z-1;
            }
            if(opt_code[i].z) references[idx] ++;
        }
    }
}
void dist_SRegs(){
    int i;
    int connections[MAXV] = {0};
    int leftnode = conf_len;
    int node;
    int cont = 0;
    int queue[MAXV] = {0},qp = 0;
    for(i = 0;i < MAXV;i ++){
        regs[i] = 0;
    }
    for(i = 0;i < conf_len;i ++){
        connections[i] = conf_sum(i);
    }
    count_refs();

    while(leftnode>0){
        node = -1;
        for(i = 0;i < conf_len;i ++){
            if(connections[i]>=0&&connections[i]<MAXSREG){
                if(node<0||references[node]<references[i])
                    node = i;
            }
        }
        if(node<0){
            for(i = 0;i < conf_len;i ++){
                if(connections[i]>=0)
                    if(node<0||references[node]>references[i])
                        node = i;
            }
            regs[node] = -1;
        }
        else{
            queue[qp++] = node;
        }
        connections[node] = -1;
        for(i = 0;i < conf_len;i ++){
            if(conflict[node][i]&&connections[i]>0){
                connections[i]--;
            }
        }
        leftnode --;
    }
    qp--;
    for(;qp >= 0;qp --){
        node = queue[qp];
        if(cont<MAXSREG){
            regs[node] = ++cont;
        }
        else{
            int neib_regs[MAXSREG] = {0};//
            for(i = 0;i < conf_len;i ++){
                if(conflict[i][node]){
                    if(regs[i]>0&&neib_regs[regs[i]-1]==0){
                        neib_regs[regs[i]-1] = 1;
                    }
                }
            }
            for(i = 0;i < MAXSREG;i ++){
                if(!neib_regs[i]){
                    regs[node] = i+1;
                    break;
                }
            }
        }
    }
    for(i = 0;i < MAXSREG;i ++){
        regocc[i] = 0;
    }
    for(i = 0;i < conf_len;i ++){
        if(regs[i]>0) regocc[regs[i]] ++;
    }
    if(debug){
        for(i = 0;i < conf_len;i ++){
            printf("%d\t",regs[i]);
        }
        printf("\n");
    }
}

void dist_tmp_reg(int bbidx){
    int i,j;
    int x,y,z;
    int end = 0;
    tmp_reg_pt = 0;
    tmp_stk_pt = 0;
    for(i = bblocklist[bbidx].opt_st;i <= bblocklist[bbidx].opt_en;i ++){
        x = -1;y=-1;z=-1;
        if(opt_code[i].flag_x==F_TMPV) x = opt_code[i].x;
        if(opt_code[i].flag_y==F_TMPV) y = opt_code[i].y;
        if(opt_code[i].flag_z==F_TMPV) z = opt_code[i].z;
        if(tmp_reg_pt<MAXTREG) end = tmp_reg_pt;
        else end = MAXTREG;
        for(j = 0;j < end;j ++){
            if(x>=0&&tmp_regs[j]==x) x=-1;
            if(y>=0&&tmp_regs[j]==y) y=-1;
            if(z>=0&&tmp_regs[j]==z) z=-1;
        }
        for(j = 0;j < tmp_stk_pt;j ++){
            if(x>=0&&tmp_stack[j]==x) x=-1;
            if(y>=0&&tmp_stack[j]==y) y=-1;
            if(z>=0&&tmp_stack[j]==z) z=-1;
        }
        if(x>=0){
            if(tmp_reg_pt<MAXTREG) tmp_regs[tmp_reg_pt++] = x;
            else if(tmp_stk_pt<MAXTMPSTK) tmp_stack[tmp_reg_pt++] = x;
            else printf("FATAL !\ttmp stack overflow\n");
        }
        if(y>=0){
            if(tmp_reg_pt<MAXTREG) tmp_regs[tmp_reg_pt++] = y;
            else if(tmp_stk_pt<MAXTMPSTK) tmp_stack[tmp_reg_pt++] = y;
            else printf("FATAL !\ttmp stack overflow\n");
        }
        if(z>=0){
            if(tmp_reg_pt<MAXTREG) tmp_regs[tmp_reg_pt++] = z;
            else if(tmp_stk_pt<MAXTMPSTK) tmp_stack[tmp_reg_pt++] = z;
            else printf("FATAL !\ttmp stack overflow\n");
        }
    }
}
void dismLocExp(){
    int i,j;
    int blks;
    opt_mips_data_seg();
    opt_emitjmain();
    for(i = 1;i <= bp;i ++){
        blks=getBaseBlock(i);
        bsc_blks = blks;
        if(debug) printf("blks=%d\n",blks);
        active(i,1);
        //if(debug) printf("act1 ok\n");
        for(j = 0;j < blks;j ++){
            bblocklist[j].opt_st = optp;
            localExpDAG(j,i);
            //if(debug) printf("st %d \ten %d\n",bblocklist[j].st,bblocklist[j].en);
            bblocklist[j].opt_en = optp-1;
        }
        //if(debug) printf("local ok\n");
        //renameActVar(i);
        //if(debug) printf("rename ok\n");
    //    opt_mips_gene(i);

        //freeActMem();
        //freeGlb();
    if(debug){
                struct udv_ *q;
        for(j= 0;j < bsc_blks;j ++){
                printf("active :\ni=%d\n",j);
                printf("st=%d\ten=%d\nins:\n",bblocklist[j].st,bblocklist[j].en);
            q = ins[j]->next;
            while(q){
                printf("%d\t",q->p->var);
                q = q->next;
            }
            printf("\nouts\n");
            q = outs[j]->next;
            while(q){
                printf("%d\t",q->p->var);
                q = q->next;
            }
            printf("\n\n");
        }
        }
        printf("%d\n",i);
    }
    //call main
    //opt_emit_callmain();
    output_opt_code4();
}
