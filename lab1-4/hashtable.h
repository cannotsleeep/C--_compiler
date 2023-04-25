#ifndef HASH
#define HASH
#include "semantic.h"
typedef struct HashNode_ HashNode;//使用之前学的Time33哈希算法
struct HashNode_{
    char *name;
    Type type;
    FieldList param;         //函数参数
    struct HashNode_ *next;  // OpneHash
};
#define CAPACITY 512
extern HashNode *gTable[CAPACITY];
extern HashNode *sTable[CAPACITY];
void Hash_copy(HashNode *h1[],HashNode *h2[]);
//注释在.c中
Type Type_get(char *name);
void glbTable();
void strcTable();

void stackinsert(char* name,int CompStdeepth,int k);
int check_new(char*name, int cdeep);
void tabledelete(char* name, int deep);
void stackclear(int compdeep);

int checksf(char* name);
int scheck(char *name);
int check(char *name);
int checkfun(Function func);
int sinsert(char *name,Type type);
int insert(char *name,Type type);
int insertFunc(Function func);
int hashFunc(char *key);
#endif
