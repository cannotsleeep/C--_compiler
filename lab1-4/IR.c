#include "IR.h"
#include "hashtable.h"
#include "semantic.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

InterCode new_InterCode(int kind){

    InterCode t = (InterCode)malloc(sizeof(struct _InterCode));
    t->kind = kind;
    return t;
}
CodeList new_CodeList(InterCode code){
    CodeList res =(CodeList)malloc(sizeof(struct _CodeList));
    res->code = code;
    res->next = res->prev = NULL;
    return res;
}

CodeList merge(CodeList c1, CodeList c2){
    if(c1 == NULL) return c2;
    if(c2 == NULL) return c1;
    CodeList t = c1;
    while(t->next != NULL){t = t->next;};
    t->next = c2;
    c2->prev = t;
    return c1;
}

CodeList CodePlus(int num, ...){
    if(num == 0){return NULL;};
    CodeList cre = NULL;
    va_list valist;
    va_start(valist, num);
    cre = va_arg(valist, CodeList); num --;
    while(cre == NULL){
        if(num <= 0 ) return NULL;
        cre = va_arg(valist, CodeList);
        num --;
    }
    while(num > 0){
        CodeList c = va_arg(valist, CodeList);
        if(c!=NULL) cre = merge (cre, c);
        num--;
    }
    va_end(valist);
    return cre;
}

CodeList Intercode(struct Node* Root){
    if(Root==NULL){return NULL;}
    if(strcmp(Root->nodeName, "Program")!= 0 ) return NULL;
    if(Root -> firstChild == NULL) return NULL;
    code_head = code_tail = NULL;
    var_tail = var_head = NULL;
    deep_cur = 0;
    var_num = 1; label_num = 1; temp_num = 1;
    Memlist_head = NULL;
    struct Node* t = Root -> firstChild;  // ExtDefList
    while(t!= NULL){
        t = t ->firstChild;
        insert_code(translate_ExtDef(t));
        t = t->Sibc;    //依次translate_ExtDef
    }
    return code_head;
}


void insert_code(CodeList codes){
    if(codes==NULL){return;}
    if(code_head==NULL){
        code_head = codes;
        CodeList t = codes;
        while (t->next!=NULL){t=t->next;}
        code_tail = t;
    } else{
        codes->prev = code_tail;
        code_tail->next = codes;
        while(code_tail->next!= NULL){code_tail= code_tail->next; }
        }
}

CodeList translate_ExtDef(struct Node* ExtDef){
    if(ExtDef->firstChild == NULL) return NULL;
    if(strcmp(ExtDef->firstChild->Sibc->nodeName, "SEMI")== 0 )return NULL;
    if(strcmp(ExtDef->firstChild->Sibc->nodeName, "ExtDecList")== 0 ) {return NULL;}
    if(checkp(ExtDef, 3, "Specifier", "FunDec", "CompSt")){
        CodeList c1 = translate_FunDec(ExtDef->firstChild->Sibc);
        CodeList c2 = translate_CompSt(ExtDef->firstChild->Sibc->Sibc);
        return merge(c1,c2);
    } else{
        fprintf(stderr, "error ExtDef!\n"); return NULL;
    }
}


CodeList translate_FunDec(struct Node* FunDec){
    if(FunDec == NULL) return NULL;
    if(checkp(FunDec, 3, "ID", "LP", "RP")){
        InterCode ic = new_InterCode(IR_FUNC);
        ic->u.func = FunDec ->firstChild -> Valstr;
        CodeList c1 = new_CodeList(ic);
        return c1;
    } else if(checkp(FunDec, 4, "ID", "LP", "VarList", "RP")){
        InterCode ic = new_InterCode(IR_FUNC);
        ic->u.func = FunDec ->firstChild -> Valstr;
        CodeList c1 = new_CodeList(ic);
        FieldList params = Type_get(FunDec->firstChild->Valstr)->u.function->param; // 寻找变量名对应的参数
        while (params!=NULL)
         {
            InterCode paramCode = new_InterCode(IR_PARAM);//PARAM节点
            Operand op;
            if(params->type->kind==BASIC){ 
                op = lookup_var(params->name); 
                op->para=1;
            }else{
                op = lookup_var(params->name); // 由于变量不重名，取名字
                op->kind=OP_ADDRESS;           //数组和结构体传递地址
                op->para=1;
                size_get(params->type);
            }
            paramCode->u.op = op;
            CodeList t=new_CodeList(paramCode);
            c1 = merge(c1,t);
            params = params->tail;
         }
        return c1;
    } else{ fprintf(stderr, "error FunDec!\n"); return NULL;}
};

CodeList translate_CompSt(struct Node* CompSt){
    if(CompSt == NULL) {return NULL;};
    if(strcmp(CompSt->firstChild->Sibc->nodeName, "RC")==0){return NULL;}
    CodeList c1 = NULL;
    CodeList c2 = NULL;
    if(strcmp(CompSt->firstChild->Sibc->nodeName, "DefList")==0){c1 = translate_DefList(CompSt->firstChild->Sibc); }
        else {c1 = translate_StmtList(CompSt->firstChild->Sibc);};
    if(strcmp(CompSt->firstChild->Sibc->Sibc->nodeName, "StmtList")==0){ 
        c2 = translate_StmtList(CompSt->firstChild->Sibc->Sibc);}
    return merge(c1, c2);
};

CodeList translate_DefList(struct Node* DefList){
    if(DefList == NULL) return NULL;
    CodeList codes = NULL;
    while(DefList!=NULL){
        struct Node* Def = DefList->firstChild; //Specifier DecList SEMI
        struct Node* Dec = Def->firstChild->Sibc->firstChild;
        codes = merge(codes,translate_Dec(Dec));
        while (Dec->Sibc!=NULL)
        {
           Dec = Dec->Sibc->Sibc->firstChild; //next Dec
           codes = merge(codes,translate_Dec(Dec));
        }
        DefList = DefList->firstChild->Sibc;
    }
    return codes;
}

CodeList translate_StmtList(struct Node* StmtList){
    if(StmtList==NULL){return NULL;}
    CodeList c1 = translate_Stmt(StmtList->firstChild);
    CodeList c2 = translate_StmtList(StmtList->firstChild->Sibc);
    return merge(c1,c2);
}

CodeList translate_Stmt(struct Node* Stmt){
    if(Stmt == NULL) return NULL;
    if(strcmp(Stmt->firstChild->nodeName, "CompSt")==0){ // CompSt
        return translate_CompSt(Stmt->firstChild);
    }else if(strcmp(Stmt->firstChild->nodeName, "Exp")==0){
        return translate_Exp(Stmt->firstChild, NULL);
    }else if(strcmp(Stmt->firstChild->nodeName, "RETURN")==0){ // RETURN
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Stmt->firstChild->Sibc,t1);
        CodeList c2 = create_code_Op(t1,IR_RETURN);
        return merge(c1,c2);
    }else if(strcmp(Stmt->firstChild->nodeName,"IF")==0){ //  IF ELSE
        if(Stmt->firstChild->Sibc->Sibc->Sibc->Sibc->Sibc==NULL){
            Operand label1 = new_label();
            Operand label2 = new_label();
            CodeList c1 = translate_Cond(Stmt->firstChild->Sibc->Sibc,label1,label2);
            CodeList c2 = translate_Stmt(Stmt->firstChild->Sibc->Sibc->Sibc->Sibc);
            CodeList clabel1 = create_code_Op(label1,IR_LABEL);
            CodeList clabel2 = create_code_Op(label2,IR_LABEL);
            return CodePlus(4,c1,clabel1,c2,clabel2);
        }else{ // IF ELSE
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            CodeList c1 = translate_Cond(Stmt->firstChild->Sibc->Sibc,label1,label2);
            CodeList c2 = translate_Stmt(Stmt->firstChild->Sibc->Sibc->Sibc->Sibc);
            CodeList c3 = translate_Stmt(Stmt->firstChild->Sibc->Sibc->Sibc->Sibc->Sibc->Sibc);
            CodeList clabel1 = create_code_Op(label1,IR_LABEL);
            CodeList clabel2 = create_code_Op(label2,IR_LABEL);
            CodeList clabel3 = create_code_Op(label3,IR_LABEL);
            CodeList goto3 = create_code_Op(label3,IR_GOTO);
            return CodePlus(7, c1, clabel1, c2, goto3, clabel2, c3, clabel3);
        }

    }else if(strcmp(Stmt->firstChild->nodeName,"WHILE")==0){ // WHILE
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        CodeList c1 = translate_Cond(Stmt->firstChild->Sibc->Sibc,label2,label3);
        CodeList c2 = translate_Stmt(Stmt->firstChild->Sibc->Sibc->Sibc->Sibc);
        CodeList goto1 = create_code_Op(label1,IR_GOTO);
        return CodePlus(6, create_code_Op(label1,IR_LABEL),c1, create_code_Op(label2,IR_LABEL), c2, goto1, create_code_Op(label3,IR_LABEL));
    }
    return NULL;
}

int size_get(Type type){
    if(type->kind == BASIC){ deep_cur=0; return 4;};
    if(type->kind == STRUCTURE) {
        int sum = 0;
        deep_cur=0;
        FieldList d = type->u.structure->domain;
        while(d != NULL){
            insert_Memlist(d->name, sum);
            sum += size_get(d->type);
            d = d ->tail;
        }
        return sum;
    }
    if(type->kind == ARRAY){
        int elemsize = size_get(type->u.array.elem);
        return (type->u.array.size * elemsize);
    }
    return 0;
}



void insert_Memlist(char* name, int val){
    Memlist m = (Memlist)malloc(sizeof(struct _Memlist));
    m->name = name;
    m->val = val;
    if(Memlist_head == NULL) {
        Memlist_head = m; 
        return;}
    Memlist tmp = Memlist_head;
    while(tmp->next!=NULL){
        tmp = tmp->next;};
    tmp->next = m;
    return;
}



Operand size_get_instruct(char* name){   
    Operand t = (Operand)malloc(sizeof(struct _Operand));
    t->kind = OP_CONSTANT;
    Memlist tnode = Memlist_head;
    while(tnode != NULL ) {
        if(strcmp(tnode->name,name)==0) {
            t->u.val = tnode->val;
            return t;
        }
        tnode = tnode-> next;
    }
    return NULL;
}



CodeList translate_Dec(struct Node *Dec){
    if(Dec==NULL) return NULL;
    struct Node* VarDec = Dec->firstChild; 
    if(Dec->firstChild->Sibc!=NULL){    // Dec -> VarDec ASSIGNOP Exp
        // VarDec ->  ID
        if(strcmp(VarDec->firstChild->nodeName,"ID")==0){
           Operand op = lookup_var(VarDec->firstChild->Valstr);
           return translate_Exp(VarDec->Sibc->Sibc, op);
        }
    }else{ // Dec->VarDec
        // VarDec -> VarDec LB INT RB | ID   
        if(strcmp(VarDec->firstChild->nodeName,"ID")==0){
            Type type = Type_get(VarDec->firstChild->Valstr); // 寻找结构体
            if(type->kind == STRUCTURE){
                Operand op = lookup_var(VarDec->firstChild->Valstr);
                op->type = type;
                op->kind = OP_STRUCT;
                InterCode ic = new_InterCode(IR_DEC);
                ic->u.dec.x = op;
                deep_cur=0;
                ic->u.dec.size = size_get(type); //DEC大小
                return new_CodeList(ic);
            }
            return NULL;
        }
        if(strcmp(VarDec->firstChild->nodeName,"VarDec")==0){ //VarDec[INT/ID]
            struct Node* IDnode = VarDec->firstChild;
            while(strcmp(IDnode->nodeName, "ID")!=0) {IDnode = IDnode->firstChild;};
            Operand var = lookup_var(IDnode->Valstr); 
            Type type = Type_get(IDnode->Valstr); //找回数组
            var->kind = OP_ARR;
            var->type = type;
            InterCode ic = new_InterCode(IR_DEC);
            ic->u.dec.x = var;
            deep_cur=0;
            ic->u.dec.size = size_get(type);
            return new_CodeList(ic);
        }
    }
    return NULL;
}

struct Node* getfather(struct Node* son, struct Node* s){
    struct Node* re = s;
    while(re->firstChild!= son && re != NULL){
        re = re->firstChild;
    };
    return re;
}

CodeList translate_Exp(struct Node* Exp, Operand place){ //place此处的值
    if( Exp == NULL ){ return NULL; }
    if(strcmp(Exp->firstChild->nodeName,"INT")==0){ //INT
        int val = Exp->firstChild->Valint;
        InterCode ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(val);
        return new_CodeList(ic);
    }
    if(strcmp(Exp->firstChild->nodeName,"ID")==0){
        if(Exp->firstChild->Sibc==NULL){ // ID
            Operand op = lookup_var(Exp->firstChild->Valstr);
            if(op->kind==OP_ARR || op->kind==OP_STRUCT ){ //数组or结构体返回地址
                InterCode ic = new_InterCode(IR_GET_ADDR);
                ic->u.assign.left = place;
                ic->u.assign.right = op;
                return new_CodeList(ic);
                
            }else{
                InterCode ic = new_InterCode(IR_ASSIGN);
                ic->u.assign.left = place;
                ic->u.assign.right = op;
                return new_CodeList(ic);
            }
        }
        if( strcmp(Exp->firstChild->Sibc->Sibc->nodeName,"RP")==0 ){ //ID()
            Function fun = Type_get(Exp->firstChild->Valstr)->u.function;
            if(strcmp(fun->name,"read")==0){
                InterCode ic = new_InterCode(IR_READ);
                ic->u.op = place;
                return new_CodeList(ic);
            }else{
                InterCode ic = new_InterCode(IR_CALL);
                if(place!=NULL){
                    ic->u.call.result = place;
                }else{
                    ic->u.call.result = new_temp();
                }
                ic->u.call.func = fun->name;
                return new_CodeList(ic); 
            }
        }
        
        if(strcmp(Exp->firstChild->Sibc->nodeName,"LP")==0){ //ID(Args)
                if( strcmp(Exp->firstChild->Sibc->Sibc->nodeName,"Args")==0 ){ 
                Function fun = Type_get(Exp->firstChild->Valstr)->u.function;
                ArgList argList = NULL;
                fdom = fun->param;
                CodeList c1 = translate_Args(Exp->firstChild->Sibc->Sibc,&argList );
                fdom = NULL;
                if(strcmp(fun->name,"write")==0){
                    InterCode ic = new_InterCode(IR_WRITE);
                    ic->u.op = argList->args;
                    return merge(c1,new_CodeList(ic));
                }else{
                    CodeList c2 = NULL;
                    while (argList!=NULL)
                    {
                        InterCode ic2 = new_InterCode(IR_ARG);
                        ic2->u.op = argList->args;
                        c2 = merge(c2,new_CodeList(ic2));
                        argList = argList->next;
                    }
                    
                    InterCode ic = new_InterCode(IR_CALL);
                    if(place!=NULL){
                        ic->u.call.result = place;
                    }else{
                        ic->u.call.result = new_temp();
                    }
                    ic->u.call.func = fun->name;
                    CodeList c3 = new_CodeList(ic);
                    return CodePlus(3,c1,c2,c3);
                }
            }
        }
        
    }
    if(strcmp(Exp->firstChild->nodeName,"MINUS")==0){ // MINUS Exp
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Exp->firstChild->Sibc,t1);
        InterCode ic = new_InterCode(IR_MINUS);
        ic->u.binop.op1 = new_constant(0);
        ic->u.binop.op2 = t1;
        ic->u.binop.result = place;
        CodeList c2 = new_CodeList(ic);
        return merge(c1,c2);
    }
    if(strcmp(Exp->firstChild->nodeName,"LP")==0){ // LP Exp RP
        return translate_Exp(Exp->firstChild->Sibc,place);
    }
    if(strcmp(Exp->firstChild->Sibc->nodeName,"ASSIGNOP")==0){ // Exp1 ASSIGNOP Exp2 
        if(strcmp(Exp->firstChild->firstChild->nodeName,"ID")==0){ // (Exp1 -> ID)
            Operand var = lookup_var(Exp->firstChild->firstChild->Valstr);
            Operand t1 = new_temp();
            CodeList c1 = translate_Exp(Exp->firstChild->Sibc->Sibc,t1);
            InterCode ic = new_InterCode(IR_ASSIGN);
            ic->u.assign.left = var;
            ic->u.assign.right = t1;
            CodeList c2 = new_CodeList(ic);
            if(place!=NULL){ // 如果需要返回值
                InterCode ic2 = new_InterCode(IR_ASSIGN);
                ic2->u.assign.left = place;
                ic2->u.assign.right = var;
                c2 = merge(c2,new_CodeList(ic2));
            }
            return merge(c1,c2);
        }else{ //赋值给数组or结构体，左侧取*
            Operand var = new_temp();
            var->kind = OP_ADDRESS;
            CodeList c0 = translate_Exp(Exp->firstChild,var);
            Operand t1 = new_temp();
            CodeList c1 = translate_Exp(Exp->firstChild->Sibc->Sibc,t1);
            InterCode ic = new_InterCode(IR_CHANGE_ADDR); 
            ic->u.assign.left = var;
            ic->u.assign.right = t1;
            CodeList c2 = new_CodeList(ic);
            if(place!=NULL){
                InterCode ic2 = new_InterCode(IR_ASSIGN);
                ic2->u.assign.left = place;
                ic2->u.assign.right = var;
                c2 = merge(c2,new_CodeList(ic2));
            }
            return CodePlus(3,c0,c1,c2);
        }
    }
    if(strcmp(Exp->firstChild->nodeName,"NOT")==0|| // NOT Exp
             strcmp(Exp->firstChild->Sibc->nodeName,"AND")==0||
             strcmp(Exp->firstChild->Sibc->nodeName,"OR")==0 ||
             strcmp(Exp->firstChild->Sibc->nodeName,"RELOP")==0){
        Operand label1 = new_label();
        Operand label2 = new_label();
        InterCode ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(0);
        CodeList c0 = new_CodeList(ic);
        CodeList c1 = translate_Cond(Exp,label1,label2);
        CodeList c2 = create_code_Op(label1,IR_LABEL);
        ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(1);
        c2 = merge(c2,new_CodeList(ic));
        CodeList c3 = create_code_Op(label2,IR_LABEL);
        return CodePlus(4,c0,c1,c2,c3);
    }
    if(strcmp(Exp->firstChild->Sibc->nodeName,"PLUS")==0 ||
             strcmp(Exp->firstChild->Sibc->nodeName,"MINUS")==0 ||
             strcmp(Exp->firstChild->Sibc->nodeName,"STAR")==0 ||
             strcmp(Exp->firstChild->Sibc->nodeName,"DIV")==0 ){  // Exp op Exp
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        CodeList c1 = translate_Exp(Exp->firstChild,t1);
        CodeList c2 = translate_Exp(Exp->firstChild->Sibc->Sibc,t2);
        InterCode ic =malloc(sizeof(struct _InterCode));
        if(strcmp(Exp->firstChild->Sibc->nodeName,"PLUS")==0){
            ic->kind = IR_PLUS;
        }else if(strcmp(Exp->firstChild->Sibc->nodeName,"MINUS")==0){
            ic->kind = IR_MINUS;
        }else if(strcmp(Exp->firstChild->Sibc->nodeName,"STAR")==0){
            ic->kind = IR_MUL;
        }else if(strcmp(Exp->firstChild->Sibc->nodeName,"DIV")==0){
            ic->kind = IR_DIV;
        }
        ic->u.binop.op1 = t1;
        ic->u.binop.op2 = t2;
        ic->u.binop.result = place;
        CodeList c3 = new_CodeList(ic);
        return CodePlus(3,c1,c2,c3);
    }
    if(strcmp(Exp->firstChild->Sibc->nodeName,"DOT")==0) { 
        Operand baseAddr = new_temp();
        baseAddr->kind = OP_ADDRESS;
        CodeList c1 = translate_Exp(Exp->firstChild, baseAddr);  //取前面的地址
        InterCode ic = new_InterCode(IR_PLUS);
        Operand tmp = new_temp();
        tmp->kind = OP_ADDRESS;
        ic->u.binop.result = tmp;
        ic->u.binop.op1 = baseAddr;
        ic->u.binop.op2 = size_get_instruct(Exp->firstChild->Sibc->Sibc->Valstr);
        CodeList c2 = new_CodeList(ic);
        InterCode ic2 = new_InterCode(IR_ASSIGN);
        ic2->u.assign.left = place;
        ic2->u.assign.right = tmp;
        CodeList c3 = new_CodeList(ic2);
        return CodePlus(3,c1,c2,c3);
    }       // Exp DOT ID 结构体
    if(strcmp(Exp->firstChild->Sibc->nodeName,"LB")==0){ // Exp1[Exp2]
        // 数组
        if(strcmp(Exp->firstChild->firstChild->nodeName, "ID")==0){
            Type type = Type_get(Exp->firstChild->firstChild->Valstr); //结构体数组和高维数组
            if(type->kind == ARRAY && (type->u.array.elem->kind == STRUCTURE  ||type->u.array.elem->kind == ARRAY)){ //结构体数组
                Operand v1 = lookup_var(Exp->firstChild->firstChild->Valstr);
                Operand baseAddr = new_temp();
                baseAddr->kind = OP_ADDRESS;
                InterCode ic;
                if(v1->kind==OP_ADDRESS){
                    ic = new_InterCode(IR_ASSIGN);
                }else{
                    ic = new_InterCode(IR_GET_ADDR);
                }
                ic->u.assign.left = baseAddr;
                ic->u.assign.right = v1;
                CodeList c0 = new_CodeList(ic);
                Operand t1 = new_temp(); // 地址偏移量
                Operand t2 = new_temp(); // 结果地址
                t2->kind=OP_ADDRESS;
                CodeList c1 = translate_Exp(Exp->firstChild->Sibc->Sibc,t1);
                ic = new_InterCode(IR_MUL);
                ic->u.binop.result = t1;
                ic->u.binop.op1 = t1;
                ic->u.binop.op2 = new_constant(size_get(type->u.array.elem));
                CodeList c2 = new_CodeList(ic);
                CodeList c3 = NULL;
                if(place!=NULL){
                    ic = new_InterCode(IR_PLUS);
                    ic->u.binop.result = t2;
                    ic->u.binop.op1 = baseAddr;
                    ic->u.binop.op2 = t1;
                    c3 = new_CodeList(ic);
                    ic = new_InterCode(IR_ASSIGN);
                    ic->u.assign.left = place;
                    ic->u.assign.right = t2;
                    c3 = merge(c3,new_CodeList(ic));
                }
                return CodePlus(4,c0,c1,c2,c3);


            }  //非结构体数组
            Operand v1 = lookup_var(Exp->firstChild->firstChild->Valstr);
            Operand baseAddr = new_temp();
            baseAddr->kind = OP_ADDRESS;
            InterCode ic;
            if(v1->kind==OP_ADDRESS){
                ic = new_InterCode(IR_ASSIGN);
            }else{
                ic = new_InterCode(IR_GET_ADDR);
            }
            ic->u.assign.left = baseAddr;
            ic->u.assign.right = v1;
            CodeList c0 = new_CodeList(ic);
            Operand t1 = new_temp(); // t1地址偏移量
            Operand t2 = new_temp(); // t2结果地址
            t2->kind=OP_ADDRESS;
            CodeList c1 = translate_Exp(Exp->firstChild->Sibc->Sibc,t1);
            ic = new_InterCode(IR_MUL);
            ic->u.binop.result = t1;
            ic->u.binop.op1 = t1;
            ic->u.binop.op2 = new_constant(4);
            CodeList c2 = new_CodeList(ic);
            CodeList c3 = NULL;
            if(place!=NULL){
                ic = new_InterCode(IR_PLUS);
                ic->u.binop.result = t2;
                ic->u.binop.op1 = baseAddr;
                ic->u.binop.op2 = t1;
                c3 = new_CodeList(ic);
                ic = new_InterCode(IR_ASSIGN);
                ic->u.assign.left = place;
                ic->u.assign.right = t2;
                c3 = merge(c3,new_CodeList(ic));
            }
            return CodePlus(4,c0,c1,c2,c3);
        }
        if(strcmp(Exp->firstChild->firstChild->Sibc->nodeName,"DOT")==0){
            Type type = Type_get(Exp->firstChild->firstChild->Sibc->Sibc->Valstr);
            struct Node* Exp1 = Exp->firstChild;
            Operand baseAddr = new_temp();
            baseAddr->kind = OP_ADDRESS;
            CodeList c0 = translate_Exp(Exp1, baseAddr);  //基地址
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            CodeList c1;
            c1 = translate_Exp(Exp->firstChild->Sibc->Sibc, t1);// Exp2
            InterCode ic;
            ic = new_InterCode(IR_MUL);
            ic->u.binop.result = t2;
            ic->u.binop.op1 = t1;
            ic->u.binop.op2 = new_constant(size_get(type->u.array.elem));
            CodeList c2 = new_CodeList(ic);  //偏移量
            Operand t3 = new_temp(); //结果地址
            t3->kind = OP_ADDRESS;
            InterCode ic2;
            ic2 = new_InterCode(IR_PLUS);
            ic2->u.binop.result = t3;
            ic2->u.binop.op1 = baseAddr;
            ic2->u.binop.op2 = t2;
            CodeList c3 = new_CodeList(ic2);
            InterCode ic3;
            ic3 = new_InterCode(IR_ASSIGN);
            ic3->u.assign.left = place;
            ic3->u.assign.right = t3;
            CodeList c4 = new_CodeList(ic3);
            return CodePlus(5,c0,c1,c2,c3,c4);
        }
        if(strcmp(Exp->firstChild->firstChild->Sibc->nodeName,"LB")==0){ //高维数组
            struct Node* Exp1 = Exp->firstChild;
            struct Node* IDnode = Exp1;
            int cnt = 0; //层数 先算出层数，然后一层一层加上
            while(strcmp(IDnode->nodeName, "ID")!=0){
                IDnode = IDnode ->firstChild;
                cnt++;
            }
            Type type = Type_get(IDnode->Valstr);
            type->u.array.deep = cnt;
            InterCode ic;
            struct Node* curNode;
            curNode = getfather( IDnode,Exp);
            Operand baseAddr = new_temp();
            baseAddr->kind = OP_ADDRESS;
            CodeList c0 = translate_Exp(curNode, baseAddr);
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            Operand t3 = new_temp();
            ic = new_InterCode(IR_ASSIGN);
            ic->u.assign.left = t3;
            ic->u.assign.right = new_constant(0);
            CodeList c1 = new_CodeList(ic);
            CodeList tmp = merge(c0,c1);
            while(cnt > 0){
                CodeList c0 = translate_Exp(curNode->Sibc->Sibc, t1);
                type = type->u.array.elem;
                ic = new_InterCode(IR_MUL);
                ic->u.binop.result = t2;
                ic->u.binop.op1 = t1;
                ic->u.binop.op2 = new_constant(size_get(type));
                CodeList c1 = new_CodeList(ic);
                tmp = CodePlus(3,tmp,c0,c1);
                ic = new_InterCode(IR_PLUS);
                ic->u.binop.result = t3;
                ic->u.binop.op1 = t3;
                ic->u.binop.op2 = t2;
                CodeList c2 = new_CodeList(ic);   //t3 = t3 + t2 记录下当前计算出的地址
                tmp = merge(tmp,c2);
                curNode = getfather( curNode ,Exp);
                cnt--;
            }
            Operand t4= new_temp();
            InterCode ic2;
            ic2 = new_InterCode(IR_PLUS);
            ic2->u.binop.result = t4;
            ic2->u.binop.op1 = t3;
            ic2->u.binop.op2 = baseAddr;
            CodeList c4 = new_CodeList(ic2);
            tmp = merge(tmp, c4);
            t4->kind = OP_ADDRESS;
            ic2 = new_InterCode(IR_ASSIGN);
            ic2->u.assign.left = place;
            ic2->u.assign.right = t4;
            CodeList c5 = new_CodeList(ic2);
            return merge(tmp, c5);

        }
            
    }
    return NULL;
}


CodeList translate_Cond(struct Node *Exp, Operand label_true, Operand label_false){
    if(strcmp(Exp->firstChild->nodeName,"NOT")==0){ // NOT EXP
        return translate_Cond(Exp->firstChild->Sibc,label_false,label_true);
    }else if(Exp->firstChild->Sibc==NULL){ // Exp
        Operand t1 = new_temp();
        CodeList c1 = translate_Exp(Exp,t1);
        InterCode ic = new_InterCode(IR_IFGOTO);
        ic->u.if_goto.x = t1;
        ic->u.if_goto.y = new_constant(0);
        ic->u.if_goto.z = label_true;
        ic->u.if_goto.relop = malloc(3);
        strcpy(ic->u.if_goto.relop, "!=");
        CodeList c2 = new_CodeList(ic);
        CodeList gf = create_code_Op(label_false,IR_GOTO);
        return CodePlus(3,c1,c2,gf);
    } else{
        if(strcmp(Exp->firstChild->Sibc->nodeName,"RELOP")==0){ //RELOP
            Operand t1 = new_temp();
            Operand t2 = new_temp();
            CodeList c1 = translate_Exp(Exp->firstChild,t1);
            CodeList c2 = translate_Exp(Exp->firstChild->Sibc->Sibc,t2);
            InterCode ic = new_InterCode(IR_IFGOTO);
            ic->u.if_goto.x = t1;
            ic->u.if_goto.y = t2;
            ic->u.if_goto.z = label_true;
            ic->u.if_goto.relop = Exp->firstChild->Sibc->Valstr;
            CodeList c3 = new_CodeList(ic);
            CodeList gf = create_code_Op(label_false,IR_GOTO);
            return CodePlus(4,c1,c2,c3,gf);
        }else if(strcmp(Exp->firstChild->Sibc->nodeName,"AND")==0){ //AND
            Operand label1 = new_label();
            CodeList c1 = translate_Cond(Exp->firstChild,label1,label_false);
            CodeList c2 = translate_Cond(Exp->firstChild->Sibc->Sibc,label_true,label_false);
            CodeList clabel1 = create_code_Op(label1,IR_LABEL);
            return CodePlus(3,c1,clabel1,c2);
        }else if(strcmp(Exp->firstChild->Sibc->nodeName,"OR")==0){ //OR
            Operand label1 = new_label();
            CodeList c1 = translate_Cond(Exp->firstChild,label_true,label1);
            CodeList c2 = translate_Cond(Exp->firstChild->Sibc->Sibc,label_true,label_false);
            CodeList clabel1 = create_code_Op(label1,IR_LABEL);
            return CodePlus(3,c1,clabel1,c2);
        }else{ // other
            Operand t1 = new_temp();
            CodeList c1 = translate_Exp(Exp,t1);
            InterCode ic = new_InterCode(IR_IFGOTO);
            ic->u.if_goto.x = t1;
            ic->u.if_goto.y = new_constant(0);
            ic->u.if_goto.z = label_true;
            ic->u.if_goto.relop = malloc(3);
            strcpy(ic->u.if_goto.relop, "!=");
            CodeList c2 = new_CodeList(ic);
            CodeList gf = create_code_Op(label_false,IR_GOTO);
            return CodePlus(3,c1,c2,gf);
        }
    }


}

CodeList translate_Args(struct Node* Args, ArgList* arg_list){
    if( Args == NULL ){ return NULL; }
    Operand t1 = new_temp();
    CodeList c1 = translate_Exp(Args->firstChild,t1);
    //返回时，根据函数参数依次访问判断，是取地址还是取值，只有数组和结构体要取地址。
    // write函数的参数为NULL，要避免空指针访问
    if(fdom!=NULL &&(fdom->type->kind == ARRAY ||
            fdom->type->kind == STRUCTURE )){t1->kind =  OP_ADDRESS;}; //当前param类型
    ArgList newArgList = malloc(sizeof(struct _ArgList));
    newArgList->args = t1;
    newArgList->next = *arg_list;
    *arg_list = newArgList;
    if(Args->firstChild->Sibc==NULL){ 
        return c1;  // Exp
    }else{ 
        if(fdom!=NULL) fdom = fdom->tail; // write函数的参数为NULL，要避免空指针访问 //next param
        CodeList c2 = translate_Args(Args->firstChild->Sibc->Sibc,arg_list);  // next Args
        return merge(c1,c2);
    }
}

CodeList create_code_Op(Operand op,int IR_KIND){
    InterCode t = new_InterCode(IR_KIND);
    t->u.op = op;
    CodeList res =new_CodeList(t);
    return res;
}

Operand lookup_var(char* varname){
    Variable vt = var_head;
    while (vt!=NULL){
        if(strcmp(vt->name,varname)==0){ return vt->op;}
        vt = vt->next;
    }
    Variable newVar = malloc(sizeof(struct _Variable));
    newVar->name = varname;
    Operand t = malloc(sizeof(struct _Operand));
    t->kind = OP_VARIABLE;
    t->u.var_no = var_num;
    var_num++;
    newVar->op = t;
    if(var_head == NULL){
        var_head = newVar;
        var_tail = var_head;
        return t;
    }
    var_tail->next = newVar;
    var_tail = newVar;
    return t;
}

Operand new_temp(){
    Operand t = malloc(sizeof(struct _Operand));
    t->kind = OP_TEMP;
    t->u.temp_no = temp_num;
    temp_num++;
    return t;
}

Operand new_label(){
    Operand t = (Operand)malloc(sizeof(struct _Operand));
    t->kind = OP_LABEL;
    t->u.temp_no = label_num;
    label_num++;
    return t;
}

Operand new_constant(int val){
    Operand t = malloc(sizeof(struct _Operand));
    t->kind = OP_CONSTANT;
    t->u.val = val;
    return t;
}

int checkp(struct Node* root, int childnum, ...){
    int re = 1;
    struct Node* t = root->firstChild;
    if(t == NULL) return 0;
    va_list valist;
    va_start(valist, childnum);
    for(int i =0 ; i< childnum; i++){
        if (t == NULL) {re = 0;break;}
        char* nodeName = va_arg(valist, char*);
        if (strcmp(nodeName, t->nodeName)!=0) {re = 0;break;}
        t = t->Sibc;
    }
    va_end(valist);
    if(t!=NULL) re = 0;
    return re;
}

char* get_Opname(Operand op){
    char tmp[64] = "";
    if(op->kind == OP_CONSTANT){
        sprintf(tmp, "#%d", op->u.val);
    }else if(op->kind == OP_LABEL){
        sprintf(tmp, "label%d", op->u.label_no);
    }else if(op->kind==OP_VARIABLE||op->kind==OP_ARR||op->kind==OP_STRUCT || op->para ==1){
        sprintf(tmp, "v%d", op->u.var_no);
    }else if(op->kind==OP_TEMP||op->kind==OP_ADDRESS){
        sprintf(tmp, "t%d", op->u.temp_no);
    }
    char *ans = malloc(strlen(tmp) + 1);
    strcpy(ans, tmp);
    return ans;
}

void print_IR(CodeList curcode,FILE* file){
    while (curcode != NULL) {
        InterCode code = curcode->code;
        switch (code->kind) {
        case IR_LABEL:
            fprintf(file, "LABEL %s :\n", get_Opname(code->u.op));
            break;
        case IR_FUNC:
            fprintf(file, "FUNCTION %s :\n", code->u.func);
            break;
        case IR_ASSIGN:
        {
            Operand l;
            l = code->u.assign.left;
            Operand r;
            r = code->u.assign.right;
            if(code->u.assign.left != NULL){
                if(l->kind==OP_ADDRESS && r->kind!=OP_ADDRESS){
                    fprintf(file, "%s := &%s\n", get_Opname(l), get_Opname(r));
                }else if(l->kind!=OP_ADDRESS && r->kind==OP_ADDRESS){
                    fprintf(file, "%s := *%s\n", get_Opname(l), get_Opname(r));
                }else{
                    fprintf(file, "%s := %s\n", get_Opname(l), get_Opname(r));
                }
            }
            break;
        }
        case IR_CHANGE_ADDR:
            if(code->u.assign.left != NULL){
                fprintf(file, "*%s := %s\n", get_Opname(code->u.assign.left), get_Opname(code->u.assign.right));}
            break;
        case IR_PLUS:
            if(code->u.binop.result != NULL){
                fprintf(file, "%s := %s + %s\n",get_Opname(code->u.binop.result), get_Opname(code->u.binop.op1), get_Opname(code->u.binop.op2));}
            break;
        case IR_MINUS:
            if(code->u.binop.result != NULL){
                fprintf(file, "%s := %s - %s\n",get_Opname(code->u.binop.result), get_Opname(code->u.binop.op1), get_Opname(code->u.binop.op2));}
            break;
        case IR_MUL:
            if(code->u.binop.result != NULL){
                fprintf(file, "%s := %s * %s\n",get_Opname(code->u.binop.result), get_Opname(code->u.binop.op1), get_Opname(code->u.binop.op2));}
            break;
        case IR_DIV:
            if(code->u.binop.result != NULL){
                fprintf(file, "%s := %s / %s\n",get_Opname(code->u.binop.result), get_Opname(code->u.binop.op1), get_Opname(code->u.binop.op2));}
            break;
        case IR_GOTO:
            fprintf(file, "GOTO %s\n", get_Opname(code->u.op));
            break;
        case IR_IFGOTO:
            fprintf(file, "IF %s %s %s GOTO %s\n", get_Opname(code->u.if_goto.x), code->u.if_goto.relop, get_Opname(code->u.if_goto.y), get_Opname(code->u.if_goto.z));
            break;
        case IR_RETURN:
            fprintf(file, "RETURN %s\n", get_Opname(code->u.op));
            break;
        case IR_DEC:
            fprintf(file, "DEC %s %d\n", get_Opname(code->u.dec.x), code->u.dec.size);
            break;
        case IR_ARG:
            fprintf(file, "ARG %s\n", get_Opname(code->u.op));
            break;
        case IR_CALL:
            fprintf(file, "%s := CALL %s\n", get_Opname(code->u.call.result), code->u.call.func);
            break;
        case IR_PARAM:
            fprintf(file, "PARAM %s\n", get_Opname(code->u.op) );
            break;
        case IR_READ:
            fprintf(file, "READ %s\n", get_Opname(code->u.op));
            break;
        case IR_WRITE:
            fprintf(file, "WRITE %s\n", get_Opname(code->u.op));
            break;
        case IR_GET_ADDR:
            fprintf(file, "%s := &%s\n", get_Opname(code->u.assign.left) , get_Opname(code->u.assign.right) );
            break;
        } ;
        curcode = curcode->next;
    }
    return ;
};
