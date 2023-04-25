#ifndef IR
#define IR
#include "node.h"
#include "semantic.h"

typedef struct _Operand* Operand;
typedef struct _InterCode* InterCode;
typedef struct _CodeList* CodeList;
typedef struct _ArgList* ArgList;
typedef struct _Variable* Variable;
typedef struct _Memlist* Memlist;

struct _Memlist{
    char* name; //成员名字
    int val;    //偏差值（这个成员的首地址在之中）
    Memlist next;
    int deepth[10];
    int deep_cur;
    int deep_raw;
};
Memlist Memlist_head;
Operand size_get_instruct(char* name);
void insert_Memlist(char* name, int val);
int deep_cur;
struct _Operand{
    enum{
        OP_VARIABLE,    //变量
        OP_CONSTANT,    //常量
        OP_ADDRESS,     //地址
        OP_LABEL,       //标签
        OP_ARR,         //数组
        OP_STRUCT,       //结构体
        OP_TEMP        //临时
    } kind;
    union{ 
        int var_no;
        int label_no;
        int val;
        int temp_no;
    } u;
    Type type;  //计算数组、结构体占用size
    int para;
};

struct _InterCode{
    enum{
        IR_ASSIGN,
        IR_LABEL,
        IR_PLUS, 
        IR_MINUS, 
        IR_MUL, 
        IR_DIV, 
        IR_FUNC, 
        IR_GOTO, 
        IR_IFGOTO, 
        IR_RET, 
        IR_DEC, 
        IR_ARG, 
        IR_CALL, 
        IR_PARAM, 
        IR_READ, 
        IR_WRITE, 
        IR_RETURN, 
        IR_CHANGE_ADDR,
        IR_GET_ADDR} kind; 
    union{
        Operand op;
        char *func;
        struct{Operand right, left; } assign;     
        struct{Operand result, op1, op2; } binop; //三地址代码
        struct{Operand x, y, z; char *relop;} if_goto;
        struct{Operand x; int size;} dec;
        struct{Operand result; char *func;} call;
    }u;
};
struct _CodeList{
    InterCode code;
    CodeList prev, next;
};

struct _ArgList{
    Operand args;
    ArgList next;
};

struct _Variable{
    char* name;
    Operand op;
    Variable next;
};

int var_num,label_num,temp_num; //新变量编号，新标签编号，新临时变量编号
CodeList code_head, code_tail;  //Intercode链表的首和尾
Variable var_head,var_tail;     //变量表的首和尾

InterCode new_InterCode(int kind);
CodeList new_CodeList(InterCode code);
CodeList merge(CodeList c1, CodeList c2);
CodeList Intercode(struct Node* Root);
void insert_code(CodeList codes);
int size_get(Type type);

CodeList translate_ExtDef(struct Node* ExtDef);
CodeList translate_FunDec(struct Node* FunDec);
CodeList translate_CompSt(struct Node* CompSt);
CodeList translate_StmtList(struct Node* StmtList);
CodeList translate_Stmt(struct Node* Stmt);
CodeList translate_DefList(struct Node* DefList);
CodeList translate_Dec(struct Node* Dec);
CodeList translate_Cond(struct Node* Exp, Operand label_true, Operand label_false);
CodeList translate_Exp(struct Node* Exp, Operand place); //!!
CodeList translate_Args(struct Node* Args, ArgList* arg_list);

Operand lookup_var(char* varname);
Operand new_temp();
Operand new_label();
Operand new_constant(int val);
CodeList create_code_Op(Operand op,int IR_KIND);
int checkp(struct Node* root, int childnum, ...);
char* get_Opname(Operand op);
void print_IR(CodeList codelists,FILE* file);

#endif