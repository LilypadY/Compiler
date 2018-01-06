#ifndef OPTIMIZE_H_INCLUDED
#define OPTIMIZE_H_INCLUDED
#include"memory.h"
#define MAXBBLK 1000
#define PMAX 100


struct basic_blk{
    int st,en;
};

struct basic_blk bblocklist[MAXBBLK];

struct DAG{
    int content;
    int index;
    struct DAG* left,*right;
    struct DAG* parents[PMAX];
    int inqueue;
    int parpt;
    struct node_list_ele* leaf_node;
    struct DAG* next,*qnext,*qlast;
};

int DAG_node_cont;

struct node_list_ele{
    int va;
    int type;
    int isinit;
    struct node_list_ele* next;
    struct DAG* node;
};

void dismLocExp();
#endif // OPTIMIZE_H_INCLUDED
