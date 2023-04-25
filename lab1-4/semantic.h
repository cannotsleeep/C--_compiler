#ifndef SEMANTIC
#define SEMANTIC
#include "node.h"
#include <string.h>

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct Structure_* Structure;
typedef struct Function_*  Function;
struct Type_{
    enum{BASIC, ARRAY, STRUCTURE, FUNCTION} kind; //变量、数组变量、函数、结构
    union{
        int basic;                                //区分基础变量类型 int1 float2
        struct {Type elem; int size; int deep;} array;      //数组采用嵌套，记录size和内部元素的情况（数组or基础变量）
        Structure structure;                      //结构体记录其他额外信息
        Function function;                        //函数也要额外记录参数
    } u;
    enum { LEFT,RIGHT,BOTH } assign; };           //左右值记录，对应错误6
struct  FieldList_{                               //变量表，可以用在函数参数、结构体内部域，链表结构
    char *name;
	Type type;
	FieldList tail;};
struct Structure_{                                //结构体记录名字和域
    char *name;
	FieldList domain;};
struct Function_{                                 //涉及到报错，记录行号参数和名字
	char *name;
	int line;
	Type type;
	FieldList param;};

FieldList fdom;
Function funclist[130];
int funclistnum;
int flagfunc;
int CompStdeepth;
struct hashstack{
    char* name;
    int deep;
};
struct hashstack* a[130][130];          //stack
int al[130];                            //stackcapacity
void insert_RW();
void semantic(struct Node* Root);
void ExtDefList(struct Node *node);
void ExtDef(struct Node *node);
void ExtDecList(struct Node *node, Type type);
Type Specifier(struct Node* node);
Type StructSpecifier(struct Node *node);    //定义好要返回类型信息
FieldList VarDec(struct Node *node, Type type,int structflag);
Function FunDec(struct Node *node, Type type);
FieldList VarList(struct Node *node);
FieldList ParamDec(struct Node *node);          //要返回参数、函数信息等
void CompSt(struct Node *node, Type ntype);
void StmtList(struct Node *node, Type ntype);
void Stmt(struct Node *node, Type ntype);
FieldList DefList(struct Node *node,int structflag);
FieldList Def(struct Node *node,int structflag);
FieldList DecList(struct Node *node, Type type,int structflag);
FieldList Dec(struct Node *node, Type type,int structflag);   //定义返回类型
Type Exp(struct Node *node);        //表达式的类型
FieldList Args(struct Node *node);  //返回域
Type Type_get_f(FieldList f, char* name);
int Type_check(Type t1,Type t2);
int Param_check(FieldList p1,FieldList p2);
int Struct_check(Structure s1,Structure s2);
void sprintError(int errorType,int lineNum, char* str);

#endif
