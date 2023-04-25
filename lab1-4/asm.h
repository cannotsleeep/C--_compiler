#ifndef ASM
#define ASM
#include "IR.h"

typedef struct _Var* Var;
typedef struct _Register* Register;

struct _Var{
	char *name;			//变量名
	int reg;			//变量在的寄存器
	int offset;			//变量在存储中的位置
	Var next;			//变量链表
};

struct _Register{
	char *name;
	Var var;
};

void print_asm(CodeList curcode,FILE* file);
void print_IR_asm(InterCode intercode);

int getReg(Operand op);
char* regname(int x);
Var findVar(char *name);
void addVar(Var var);
void setReg(int regIndex,Var var);

void swOp(int x);

#endif