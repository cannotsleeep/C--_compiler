#include "asm.h"

Register r[32];
Var varlist = NULL;
int curReg = 0;
int paramNum = 0;
int argNum = 0;
int sp_offset = 0;
FILE* filex;

void print_asm(CodeList curcode,FILE* file){
    filex = file;
    //head
    fprintf(filex,".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\n");
    //read
	fprintf(filex,"\nread:\n");
	fprintf(filex,"\tli $v0, 4\n");
	fprintf(filex,"\tla $a0, _prompt\n");
	fprintf(filex,"\tsyscall\n");
	fprintf(filex,"\tli $v0, 5\n");
	fprintf(filex,"\tsyscall\n");
	fprintf(filex,"\tjr $ra\n");
    //write
	fprintf(filex,"\nwrite:\n");
	fprintf(filex,"\tli $v0, 1\n");
	fprintf(filex,"\tsyscall\n");
	fprintf(filex,"\tli $v0, 4\n");
	fprintf(filex,"\tla $a0, _ret\n");
	fprintf(filex,"\tsyscall\n");
	fprintf(filex,"\tmove $v0, $0\n");
	fprintf(filex,"\tjr $ra\n\n");

    for(int i = 0; i<32; i++){
        Register x = (Register)malloc(sizeof(struct _Register));
        x->name = regname(i);
        x->var = NULL;
        r[i] = x;
    }
    while(curcode != NULL){
        print_IR_asm(curcode->code);
        curcode = curcode->next;
    }
}


void print_IR_asm(InterCode intercode){
    switch(intercode->kind){
    case IR_LABEL:
        fprintf(filex, "%s:\n", get_Opname(intercode->u.op));
        break;
    case IR_FUNC:
        fprintf(filex, "%s:\n",intercode->u.func);
        fprintf(filex, "\tsubu $sp, $sp, 4\n");
        fprintf(filex, "\tsw $fp, 0($sp)\n");
        fprintf(filex, "\tmove $fp, $sp\n");
        fprintf(filex, "\tsubu $sp, $sp, %d\n",1000);
        sp_offset = 0;
        paramNum = 0;
        break;
    case IR_ASSIGN:
    {
        Operand left = intercode->u.assign.left;
        Operand right = intercode->u.assign.right;
        if(right->kind == OP_CONSTANT){           // x := #k
            int x = getReg(left);
            fprintf(filex, "\tli %s, %d\n", regname(x), right->u.val);
            swOp(x);
            break;
        };
        if(left->kind==OP_ADDRESS && right->kind!=OP_ADDRESS)  { // x:= &y
            int x = getReg(left);
            int y = getReg(right);
            fprintf(filex, "\tmove %s, %s\n", regname(x), regname(y));
            swOp(x);
            break;
        }
        if(left->kind!=OP_ADDRESS && right->kind==OP_ADDRESS){ // x := *y
            int x = getReg(left);
            int y = getReg(right);
            fprintf(filex, "\tlw %s, 0(%s)\n", regname(x), regname(y));
            swOp(x);
            break;
        }
        int x = getReg(left);
        int y = getReg(right);
        fprintf(filex, "\tmove %s, %s\n", regname(x), regname(y));
        swOp(x);
        break;
    }
    case IR_CHANGE_ADDR:
    {
        Operand left = intercode->u.assign.left;
        Operand right = intercode->u.assign.right;
        int x = getReg(left);
        int y = getReg(right);
        if(right->kind == OP_CONSTANT){
            fprintf(filex, "\tli  %s, %d\n", regname(y) , right->u.val);
        }
        fprintf(filex, "\tsw %s, 0(%s)\n", regname(y), regname(x));
        break;
    }
    case IR_PLUS:
    case IR_MINUS:
    case IR_MUL:
    case IR_DIV:
    {
        Operand re = intercode->u.binop.result;
        Operand l = intercode->u.binop.op1;
        Operand r = intercode->u.binop.op2;
        int x,y,z;
        x = getReg(re);
        y = getReg(l);
        z = getReg(r);
        //常数依旧会返回一个寄存器，把常数保存到这个寄存器里
        if(l->kind==OP_CONSTANT){
            fprintf(filex,"\tli %s, %d\n", regname(y), l->u.val);};
        if(r->kind==OP_CONSTANT){
            fprintf(filex,"\tli %s, %d\n", regname(z), r->u.val);};
        switch (intercode->kind){
            case IR_PLUS:
                fprintf(filex,"\tadd %s, %s, %s\n", regname(x), regname(y), regname(z));
                break;
            case IR_MINUS:
                fprintf(filex,"\tsub %s, %s, %s\n", regname(x), regname(y), regname(z));
                break;
            case IR_DIV:
                fprintf(filex,"\tdiv %s, %s\n", regname(y), regname(z));
                fprintf(filex,"\tmflo %s\n", regname(x));
                break;
            case IR_MUL:
                fprintf(filex,"\tmul %s, %s, %s\n", regname(x), regname(y), regname(z));
                break;
            default:
                break;
        }
        swOp(x);
        break;
    }
    case IR_GOTO:
    {
        char* des = get_Opname(intercode->u.op);
        fprintf(filex, "\tj %s\n", des);
        break;
    }
    case IR_IFGOTO:
        //_todo
    {
        Operand leftOp = intercode->u.if_goto.x;
        Operand rightOp = intercode->u.if_goto.y;
        char* op =intercode->u.if_goto.relop;
        char *z = get_Opname(intercode->u.if_goto.z); //label
        int x = getReg(leftOp);
        int y = getReg(rightOp);  //常数依旧会返回一个寄存器，把常数保存到这个寄存器里
        if(leftOp->kind == OP_CONSTANT)  fprintf(filex,"\tli %s, %d\n", regname(x), leftOp->u.val);
        if(rightOp->kind == OP_CONSTANT)  fprintf(filex,"\tli %s, %d\n", regname(y), rightOp->u.val);
        if(strcmp(op, "==")==0){
                fprintf(filex, "\tbeq %s, %s, %s\n",regname(x), regname(y), z);
            } else if(strcmp(op, "!=")==0){
                fprintf(filex, "\tbne %s, %s, %s\n",regname(x), regname(y), z);
            } else if(strcmp(op, ">")==0){
                fprintf(filex, "\tbgt %s, %s, %s\n",regname(x), regname(y), z);
            } else if(strcmp(op, "<")==0){
                fprintf(filex, "\tblt %s, %s, %s\n",regname(x), regname(y), z);
            } else if(strcmp(op, ">=")==0){
                fprintf(filex, "\tbge %s, %s, %s\n",regname(x), regname(y), z);
            } else if(strcmp(op, "<=")==0){
                fprintf(filex, "\tble %s, %s, %s\n",regname(x), regname(y), z);
            }  
        break;
    }
    case IR_RETURN:
    {
        Operand re = intercode->u.op;
        int x;
        if(re->kind == OP_CONSTANT){
            fprintf(filex,"\tli $v0, %d\n", re->u.val);
        }  else{
            x = getReg(re);
            fprintf(filex, "\tmove $v0, %s\n", regname(x));
        }
        fprintf(filex, "\taddi $sp, $sp, %d\n", 1000);
        fprintf(filex, "\tlw $fp, 0($sp)\n");
        fprintf(filex, "\taddi $sp, $sp, 4\n");
        fprintf(filex, "\tjr $ra\n");
        if(re->kind!=OP_CONSTANT) swOp(x);
        break;
    }
    case IR_DEC:
        //分配空间
    {
        Var arr = malloc(sizeof(struct _Var));
        sp_offset -= 4;
        arr->offset = sp_offset;
        arr->next =NULL;
        sp_offset -= intercode->u.dec.size;
        arr->name = get_Opname(intercode->u.dec.x);;
        addVar(arr);
        fprintf(filex,"\taddi $s1, $fp, %d\n", sp_offset);
        fprintf(filex,"\tsw $s1, %d($fp)\n", arr->offset);
        break;
    }
    case IR_ARG:
    {
        Var arg = findVar(get_Opname(intercode->u.op));
        fprintf(filex, "\tlw $s0, %d($fp)\n", arg->offset);
        fprintf(filex, "\tsubu $sp, $sp, 4\n");
        fprintf(filex, "\tsw $s0, 0($sp)\n");
        argNum++;
        break;
    }
    case IR_CALL:
    {   
        fprintf(filex, "\tli $v1,%d\n",argNum*4); 
        fprintf(filex, "\tsubu $sp, $sp, 4\n");
        fprintf(filex, "\tsw $v1, 0($sp)\n"); 
        argNum = 0; //调用前处理ARG空间
        fprintf(filex,"\tsubu $sp, $sp, 4\n");
        fprintf(filex,"\tsw $ra, 0($sp)\n");
        int x = getReg(intercode->u.call.result);
        fprintf(filex, "\tjal %s\n", intercode->u.call.func);
        fprintf(filex, "\tmove %s, $v0\n", regname(x));
        swOp(x);
        fprintf(filex,"\tlw $ra, 0($sp)\n");
        fprintf(filex,"\taddi $sp, $sp, 4\n");
        fprintf(filex,"\tlw $v1, 0($sp)\n"); 
        fprintf(filex,"\taddi $sp, $sp, 4\n");
        fprintf(filex,"\tadd $sp, $sp, $v1\n");
        break;
    }
    case IR_PARAM:
    {
        //_todo
        sp_offset-=4;
        Var param = malloc(sizeof(struct _Var));
        param->name = get_Opname(intercode->u.op);
        sp_offset -= 4;
        param->offset = sp_offset;
        param->next = NULL;
        addVar(param);
        fprintf(filex, "\tlw $a0, %d($fp)\n", (paramNum+3)*4);
        fprintf(filex, "\tsw $a0, %d($fp)\n", param->offset);
        paramNum++;
        break;
    }
    case IR_READ:
    {
        fprintf(filex, "\taddi $sp, $sp, -4\n");
        fprintf(filex, "\tsw $ra, 0($sp)\n");
        fprintf(filex, "\tjal read\n");
        int x = getReg(intercode->u.op);
        fprintf(filex, "\tmove %s, $v0\n",regname(x));
        swOp(x);
        fprintf(filex, "\tlw $ra, 0($sp)\n");
        fprintf(filex, "\taddi $sp, $sp, 4\n");
        break;
    }
    case IR_WRITE:
    {
        fprintf(filex,"\taddi $sp, $sp, -4\n");
        fprintf(filex,"\tsw $ra, 0($sp)\n");
        int x = getReg(intercode->u.op);
        if(intercode->u.op->kind==OP_VARIABLE || intercode->u.op->kind ==OP_TEMP){
            fprintf(filex,"\tmove $a0, %s\n", regname(x));
        }else if(intercode->u.op->kind ==OP_ADDRESS || intercode->u.op->kind == OP_STRUCT || intercode->u.op->kind == OP_ARR){
            fprintf(filex, "\tlw $a0, 0(%s)\n", regname(x));
        }
        fprintf(filex,"\tjal write\n");
        swOp(x);
        fprintf(filex,"\tlw $ra, 0($sp)\n");
        fprintf(filex,"\taddi $sp, $sp, 4\n");
        break;
    }
    case IR_GET_ADDR:
    {   
        int x = getReg(intercode->u.assign.left);
        int y = getReg(intercode->u.assign.right);
        fprintf(filex, "\tmove %s, %s\n", regname(x), regname(y));
        swOp(x);
        break;
    }
    return ;
    }
}




int getReg(Operand op){
    //常数同样返回一个新寄存器
    char* name = get_Opname(op);
    Var var = findVar(name);
    int i = curReg + 8;
    curReg = (curReg+1)%8;
    if(var == NULL){
        var = malloc(sizeof(struct _Var));
        var->next = NULL;
        var->name = name;
        sp_offset -= 4;
        var->offset = sp_offset;
        addVar(var);
        setReg(i,var);
    }
    else {
        setReg(i, var);
        fprintf(filex, "\tlw %s, %d($fp)\n", regname(i), var->offset);
    }
    return i;
}



char* regname(int x){
    //fprintf(stderr,"Error test!  %d\n",x);
    switch(x){
        case 0: return "$zero";
        case 1: return "$at";
        case 2: return "$v0";
        case 3: return "$v1";
        case 4 ... 7: {
            //fprintf(stderr,"Error test!  %d\n",x);
            //char* re = "$a";
            char* re = (char*)malloc(3*sizeof(char)+1);
            re[0]='$';
            re[1]='a';
            re[2]=(char)(x+44);
            return re;
            }
        case 8 ... 15:{
            char* re = (char*)malloc(3*sizeof(char)+1);
            re[0]='$';
            re[1]='t';
            re[2]=(char)(x+40);
            return re;
            }
        case 16 ... 23:{
            char* re = (char*)malloc(3*sizeof(char)+1);
            re[0]='$';
            re[1]='s';
            re[2]=(char)(x+32);
            return re;
            }
        case 24: return "$t8";
        case 25: return "$t9";
        case 26: return "$k0";
        case 27: return "$k1";
        case 28: return "$gp";
        case 29: return "$sp";
        case 30: return "$fp";
        case 31: return "$ra";
    }
}

Var findVar(char *name){
    Var varx = varlist;
    while (varx!=NULL)
    {
        if(strcmp(varx->name, name)==0){return varx;}
        varx = varx->next;
    }
    return NULL;
}

void addVar(Var var){
    Var varx = varlist;
    if(varx==NULL){varlist = var;return ;}
    while (varx->next!=NULL) {varx = varx->next;}
    varx->next = var;
}

void setReg(int i, Var var){
    var->reg = i;
    r[i]->var = var;
}


void swOp(int x){
    Var var = r[x]->var;
	fprintf(filex, "\tsw %s, %d($fp)\n", regname(x), var->offset);
    return; 
}