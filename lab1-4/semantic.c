#include "semantic.h"
#include "hashtable.h"

void iniOp(){
    for(int i = 0;i<64;i++){
        al[i]=0;
    }
    CompStdeepth=0;
    return;
}

void declareFunc(Function func){
    if(func == NULL) return;
    for(int i =0; i<funclistnum; i++){
        if(strcmp(funclist[i]->name, func->name)==0){
            if(Type_check(funclist[i]->type,func->type)==0 || Param_check(funclist[i]->param,func->param) ==0){
                sprintError(19,func->line, func->name);
            }
            return;
        }
    }
    funclist[funclistnum] = func;
    funclistnum++;
    return ;
};

int undeclareFunc(Function func){
    if(func == NULL) return 1;
    for(int i =0; i<funclistnum; i++){
        if(strcmp(funclist[i]->name, func->name)==0){
            Function func1 = (Function)malloc(sizeof(struct Function_));
            func1->type = funclist[i]->type;
            func1->param = funclist[i]->param;
            for(int j = i; j<funclistnum-1; j++){ funclist[j] = funclist[j+1] ; };
            funclist[funclistnum-1] = NULL;
            funclistnum = funclistnum - 1 ;
            if(Type_check(func1->type,func->type)==0 || Param_check(func1->param,func->param) ==0){
                sprintError(19,func->line, func->name);
                return 0;
            }
            return 1;
        }
    }
    return 1;
};

void DecFun_check(){
    if(funclistnum == 0) return;
    for(int i = 0; i<funclistnum;i++){
        int tt = checkfun(funclist[i]);
        if(tt==0) {sprintError(18,funclist[i]->line, funclist[i]->name);}
            else if(tt ==2)sprintError(19,funclist[i]->line, funclist[i]->name);

    }
}

void insert_RW(){
    Function read = (Function)malloc(sizeof(struct Function_));
    Function write = (Function)malloc(sizeof(struct Function_));
    read->name = "read";
    write->name = "write";
    read->line = 0;
    write->line = 0;
    Type type= (Type)malloc(sizeof(struct Type_));
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
    type->kind = BASIC;
    read->type = type;
    write->type = type;
    type->u.basic = 1;
    f->type = type;
    write->param = f;
    insertFunc(read);
    insertFunc(write);
    return ;
}
void semantic(struct Node* Root){
    funclistnum = 0;
    iniOp();
    insert_RW();
    ExtDefList(Root->firstChild);
    DecFun_check();
}

void ExtDefList(struct Node *node){
    if(node==NULL)
		return;
    if(node->firstChild==NULL){
        return;
    }else{
        ExtDef(node->firstChild);
        ExtDefList(node->firstChild->Sibc);
    }
}

void ExtDef(struct Node* node){
    if(node == NULL) return;
    Type t = Specifier(node->firstChild);
    struct Node *tmp = node->firstChild->Sibc;
    if(strcmp(tmp->nodeName,"ExtDecList")==0){ 
        ExtDecList(tmp, t);
    }else if(strcmp(tmp->nodeName,"SEMI")==0){ }
    else if(strcmp(tmp->nodeName,"FunDec")==0){
        if(strcmp(tmp->Sibc->nodeName,"SEMI") == 0){
            flagfunc = 1;
            Function func = FunDec(tmp, t);
            if(func == NULL){return;};
            declareFunc(func);
            flagfunc = 0;
        }else if(strcmp(tmp->Sibc->nodeName,"CompSt")==0){
            CompStdeepth++;
            Function func = FunDec(tmp, t);
            if(func == NULL){return;};
            if(undeclareFunc(func)==1){
                insertFunc(func);
                CompStdeepth--;
                CompSt(tmp->Sibc, t);
            };

        }
    }
}

void ExtDecList(struct Node *node, Type type){
	if(node==NULL) return;
    VarDec(node->firstChild,type,0);
    if(node->firstChild->Sibc!=NULL){ 
        ExtDecList(node->firstChild->Sibc->Sibc,type);
    }
} 

Type Specifier(struct Node* node){
    if(node == NULL) return NULL;
    Type type;
    if(strcmp(node->firstChild->nodeName, "TYPE" )==0){
        type = (Type)malloc(sizeof(struct Type_));
        type->kind = BASIC;
        if(strcmp(node->firstChild->Valstr, "int")==0){
            type->u.basic = 1;
        };
        if(strcmp(node->firstChild->Valstr, "float")==0){
            type->u.basic = 2;
        };
    }
    else if(strcmp(node->firstChild->nodeName,"StructSpecifier")==0) { 
        type = StructSpecifier(node->firstChild);
    }
    return type;
}

Type StructSpecifier(struct Node *node){
    if(node==NULL)  return NULL;  
    struct Node* tmp = node->firstChild->Sibc;
    if(strcmp(tmp->nodeName,"OptTag")==0){ 
        Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = STRUCTURE;
		Structure structure = (Structure)malloc(sizeof(struct Structure_));
		type->u.structure = structure;
        if(tmp==NULL){ structure->name = NULL;
        }else{     structure->name = tmp->firstChild->Valstr;   }
        HashNode *backup[CAPACITY];
        Hash_copy(sTable,backup);
        strcTable();
        structure->domain = DefList(tmp->Sibc->Sibc,1);     
        Hash_copy(backup,sTable);
        type->assign = BOTH;
        if(structure->name!=NULL){
            if(check(structure->name)==1){
                sprintError(16,tmp->firstChild->lineNum,structure->name);
                return NULL;
            }else{
                insert(structure->name,type);
            }
        }
        return type;
    }else if(strcmp(tmp->nodeName,"LC")==0){
        Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = STRUCTURE;
		Structure structure = (Structure)malloc(sizeof(struct Structure_));
		type->u.structure = structure;
        structure->name = NULL;
        HashNode *backup[CAPACITY];
        Hash_copy(sTable,backup);
        strcTable();
        structure->domain = DefList(tmp->Sibc,1); 
        Hash_copy(backup,sTable);
        type->assign = BOTH;
        if(structure->name!=NULL){
            if(check(structure->name)==1){
                sprintError(16,tmp->firstChild->lineNum,structure->name);
                return NULL;
            }else{
                insert(structure->name,type);
            }
        }
        return type;
    }
        else{ 
        Type type = Type_get(tmp->firstChild->Valstr);
        //fprintf(stderr, "Get: %d\n", Type_get("a")==NULL);
        if(type==NULL){
            sprintError(17,tmp->firstChild->lineNum,tmp->firstChild->Valstr);
            return NULL;
        }else{
            return type;
        }

    };
}

FieldList VarDec(struct Node* node, Type type,int structflag){
    if(node==NULL)	return NULL;  
    if(strcmp(node->firstChild->nodeName,"ID")==0){
        if(structflag == 0 && (check_new(node->firstChild->Valstr,CompStdeepth)==1 || checksf(node->firstChild->Valstr)==1)){
            sprintError(3,node->firstChild->lineNum,node->firstChild->Valstr);
	        FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
            f->name = node->firstChild->Valstr;
            f->type = type;
            f->tail = NULL;
            return f;
        }else if(structflag==1&&(check(node->firstChild->Valstr)==1||scheck(node->firstChild->Valstr)==1)){
            sprintError(15,node->firstChild->lineNum,node->firstChild->Valstr);
            FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
            f->name = node->firstChild->Valstr;
            f->type = type;
            f->tail = NULL;
            return f;
        }else{
	        FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
            if(structflag==0 && flagfunc == 0){
                int k;
                k = insert(node->firstChild->Valstr,type); 
                stackinsert(node->firstChild->Valstr,CompStdeepth,k);
            }else{
                sinsert(node->firstChild->Valstr,type);
                insert(node->firstChild->Valstr,type);
            }
            f->name = node->firstChild->Valstr;
            f->type = type;
            f->tail = NULL;
            return f;
        }
    }else{
        Type varDec = (Type)malloc(sizeof(struct Type_));
		varDec->kind = ARRAY;
		varDec->u.array.size = node->firstChild->Sibc->Sibc->Valint;
		varDec->u.array.elem = type; 
        FieldList fl = VarDec(node->firstChild,varDec,structflag);
        return fl;
    }
}


Function FunDec(struct Node *node, Type type){
    if(node==NULL) return NULL;
    Function func = (Function)malloc(sizeof(struct Function_));
    func->name = node->firstChild->Valstr;
    func->line = node->firstChild->lineNum;
    func->type = type;
    if(checkfun(func)!=0 && flagfunc == 0){
        sprintError(4,node->firstChild->lineNum,node->firstChild->Valstr);
        return NULL;
    }
    
    if(strcmp(node->firstChild->Sibc->Sibc->nodeName,"VarList")==0){
        FieldList f = VarList(node->firstChild->Sibc->Sibc);
        func->param = f;
    }else{
        func->param = NULL;
    }
    return func;
}

FieldList VarList(struct Node* node){
	if(node==NULL)	return NULL;
    FieldList f = ParamDec(node->firstChild);
    if(node->firstChild->Sibc!=NULL){
        if(f==NULL){
            return f = VarList(node->firstChild->Sibc->Sibc);
        }else{
            f->tail = VarList(node->firstChild->Sibc->Sibc);
        }
    }
    return f;
}

FieldList ParamDec(struct Node *node){
	if(node==NULL) return NULL;
    Type t = Specifier(node->firstChild);
    return VarDec(node->firstChild->Sibc,t,0);
}

void CompSt(struct Node *node, Type ntype){
    CompStdeepth++;
    if(node==NULL)	return;
    if(strcmp(node->firstChild->Sibc->nodeName,"DefList")==0){
        DefList(node->firstChild->Sibc,0);
        if(strcmp(node->firstChild->Sibc->Sibc->nodeName,"StmtList")==0){
            StmtList(node->firstChild->Sibc->Sibc,ntype);};
    }
        else if(strcmp(node->firstChild->Sibc->nodeName,"StmtList")==0){
            StmtList(node->firstChild->Sibc,ntype);
        };
    stackclear(CompStdeepth);
    CompStdeepth--;
}

void StmtList(struct Node *node, Type ntype){
    if(node==NULL)		return;
    if(node->firstChild==NULL){   return; }
        else{ 
            Stmt(node->firstChild,ntype);
            StmtList(node->firstChild->Sibc,ntype);
    }
}

void Stmt(struct Node *node, Type ntype){
    if(node==NULL)	return;
    if(strcmp(node->firstChild->nodeName,"Exp")==0){ 
        Exp(node->firstChild);
    }else if(strcmp(node->firstChild->nodeName,"CompSt")==0){ 
        CompSt(node->firstChild,ntype);
    }else if(strcmp(node->firstChild->nodeName,"RETURN")==0){
        Type expType = Exp(node->firstChild->Sibc);
		if(expType==NULL) return; 
        if(!Type_check(ntype,expType)){
            sprintError(8,node->firstChild->lineNum,"");
        }
    }else if(strcmp(node->firstChild->nodeName,"WHILE")==0){
        Exp(node->firstChild->Sibc->Sibc);
        Stmt(node->firstChild->Sibc->Sibc->Sibc->Sibc,ntype);
    }else if(strcmp(node->firstChild->nodeName,"IF")==0){
        Exp(node->firstChild->Sibc->Sibc);
        Stmt(node->firstChild->Sibc->Sibc->Sibc->Sibc,ntype);
        if(node->firstChild->Sibc->Sibc->Sibc->Sibc->Sibc!=NULL){
            Stmt(node->firstChild->Sibc->Sibc->Sibc->Sibc->Sibc->Sibc,ntype);
        }
    }
}

FieldList DefList(struct Node *node, int structflag){
    if(node==NULL) return NULL;
    if(strcmp(node->nodeName,"DefList")!=0) return NULL;
    if(strcmp(node->firstChild->nodeName, "Def")!=0){return NULL;}
    else{
        FieldList f = Def(node->firstChild,structflag);
        if(f == NULL){
            f = DefList(node->firstChild->Sibc,structflag);
        }else{
            FieldList tmp = f;
            FieldList deflist  = DefList(node->firstChild->Sibc,structflag);
            while (tmp->tail!=NULL)
            {
                tmp = tmp->tail;
            }
            tmp->tail = deflist;
        }
        return f;
    }
}

FieldList Def(struct Node *node,int structflag){
	if(node==NULL) return NULL;
    Type t = Specifier(node->firstChild);
    return DecList(node->firstChild->Sibc,t,structflag);
}

FieldList DecList(struct Node *node, Type type, int structflag){

	if(node==NULL) return NULL;
    FieldList f = Dec(node->firstChild,type,structflag);
    if(f == NULL){ return NULL; }
    if(node->firstChild->Sibc!=NULL){
        FieldList tmp = f;
        FieldList declist  = DecList(node->firstChild->Sibc->Sibc,type,structflag);
        while (tmp->tail!=NULL)
        {
            tmp = tmp->tail;
        }
        tmp->tail = declist;
    }
    return f;
}

FieldList Dec(struct Node *node, Type type,int structflag){
	if(node==NULL)return NULL;
    FieldList f = VarDec(node->firstChild,type,structflag);
    if(node->firstChild->Sibc!=NULL){
        if(structflag==1){
            sprintError(15,node->firstChild->lineNum,"");
            return NULL;
        }
        Type expType = Exp(node->firstChild->Sibc->Sibc);
        if(!Type_check(type, expType)){
            sprintError(5,node->firstChild->Sibc->lineNum,NULL);
            return NULL;
        }
    }
    return f;
}

Type Exp(struct Node *node){
	if(node==NULL) return NULL;
    if(strcmp(node->firstChild->nodeName,"INT")==0){
        Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->u.basic = 1;
		type->assign = RIGHT;
		return type;
    }else if(strcmp(node->firstChild->nodeName,"FLOAT")==0){
		Type type = (Type)malloc(sizeof(struct Type_));
		type->kind = BASIC;
		type->u.basic = 2;
		type->assign = RIGHT;
		return type;
    }else if( strcmp(node->firstChild->nodeName,"ID")==0 ){
        if(node->firstChild->Sibc==NULL){
            if(check(node->firstChild->Valstr)==0){
                sprintError(1,node->firstChild->lineNum, node->firstChild->Valstr);
                return NULL;
            }else{
                Type type =Type_get(node->firstChild->Valstr);
                return type;
            }
        }else if(strcmp(node->firstChild->Sibc->Sibc->nodeName,"Args")==0){
            Type funType = Type_get(node->firstChild->Valstr);
            if(funType==NULL){
                sprintError(2,node->firstChild->lineNum,node->firstChild->Valstr);
                return NULL;
            }else if(funType->kind!=FUNCTION){
                sprintError(11,node->firstChild->lineNum,"");
                return NULL;
            }else{
                FieldList param = Args(node->firstChild->Sibc->Sibc);
                if(!Param_check(param,funType->u.function->param)){
                    sprintError(9, node->firstChild->lineNum, node->firstChild->Valstr);
                    return NULL;
                }
                Type res = (Type)malloc(sizeof(struct Type_));
                memcpy(res, funType->u.function->type, sizeof(struct Type_));
                res->assign = RIGHT; 
                return res;
            }
        }else{
            Type funType = Type_get(node->firstChild->Valstr);
            if(funType==NULL){
                sprintError(2,node->firstChild->lineNum,node->firstChild->Valstr);
                return NULL;
            }else if(funType->kind!=FUNCTION){
                sprintError(11,node->firstChild->lineNum,"");
                return NULL;
            }else if(funType->u.function->param!=NULL){
                sprintError(9,node->firstChild->lineNum,node->firstChild->Valstr);
                return NULL;
            }
            
            Type res = (Type)malloc(sizeof(struct Type_));
            memcpy(res, funType->u.function->type, sizeof(struct Type_));
            res->assign = RIGHT; 
            return res;
        }
     }else if(strcmp(node->firstChild->nodeName,"MINUS")==0){
        Type type = Exp(node->firstChild->Sibc);
        if(type==NULL){
            return NULL;
        }
        if(type->kind!=BASIC){
            sprintError(7,node->firstChild->Sibc->lineNum,"");
            return NULL;
        }
        Type res = (Type)malloc(sizeof(struct Type_));
        memcpy(res, type, sizeof(struct Type_));
        res->assign = RIGHT;
        return res;
     }else if(strcmp(node->firstChild->nodeName,"NOT")==0){ 
        Type type =  Exp(node->firstChild->Sibc);
        if(type==NULL){
            return NULL;
        }
        Type res = (Type)malloc(sizeof(struct Type_));
        memcpy(res, type, sizeof(struct Type_));
        res->assign = RIGHT;
        return res;
     }else if(strcmp(node->firstChild->nodeName,"LP")==0){
        Type type =  Exp(node->firstChild->Sibc);
        if(type==NULL){
            return NULL;
        }
        Type res = (Type)malloc(sizeof(struct Type_));
        memcpy(res, type, sizeof(struct Type_));
        res->assign = RIGHT;
        return res;
     }else if(strcmp(node->firstChild->Sibc->nodeName,"ASSIGNOP")==0){ 
        Type left = Exp(node->firstChild);
        Type right = Exp(node->firstChild->Sibc->Sibc);
        if(left==NULL||right==NULL){
            return NULL;
        }
        if(left->assign==RIGHT){
            sprintError(6,node->firstChild->lineNum,"");
		    return NULL;
		}
        if(!Type_check(left,right)){
            sprintError(5,node->firstChild->Sibc->lineNum,"");
            return NULL;
        }
        return left;
    }else if(strcmp(node->firstChild->Sibc->nodeName,"AND")==0||
             strcmp(node->firstChild->Sibc->nodeName,"OR")==0 ||
             strcmp(node->firstChild->Sibc->nodeName,"RELOP")==0 ||
             strcmp(node->firstChild->Sibc->nodeName,"PLUS")==0 ||
             strcmp(node->firstChild->Sibc->nodeName,"MINUS")==0 ||
             strcmp(node->firstChild->Sibc->nodeName,"STAR")==0 ||
             strcmp(node->firstChild->Sibc->nodeName,"DIV")==0 ){
        Type left = Exp(node->firstChild);
        Type right = Exp(node->firstChild->Sibc->Sibc);
        if(left==NULL||right==NULL){
            return NULL;
        }
        if(Type_check(left,right)==0){
           sprintError(7,node->firstChild->lineNum,"");
           return NULL;
        }
        if(strcmp(node->firstChild->Sibc->nodeName,"RELOP")==0){
			Type res = (Type)malloc(sizeof(struct Type_));
            res->kind = BASIC;
            res->assign = RIGHT;
            res->u.basic = 1;
            return res;
        }else{
            Type res = (Type)malloc(sizeof(struct Type_));
            memcpy(res, left, sizeof(struct Type_));
            res->assign = RIGHT;
            return res;
        }
    }else if(strcmp(node->firstChild->Sibc->nodeName,"DOT")==0) { 
        Type left = Exp(node->firstChild);
        if(left==NULL){ 
            return NULL;
        }
        if(left->kind!=STRUCTURE){
            sprintError(13,node->firstChild->lineNum,"");
            return NULL;
        }else{
            Type type = Type_get_f(left->u.structure->domain,node->firstChild->Sibc->Sibc->Valstr);
            if(type==NULL){
                sprintError(14,node->firstChild->Sibc->Sibc->lineNum,node->firstChild->Sibc->Sibc->Valstr);
                return NULL;
            }
            return type;
        }
    }else if(strcmp(node->firstChild->Sibc->nodeName,"LB")==0){
        Type left = Exp(node->firstChild);
        Type right = Exp(node->firstChild->Sibc->Sibc);
        if(left==NULL||right==NULL){ 
            return NULL;
        }
        if(left->kind!=ARRAY){
            sprintError(10,node->firstChild->lineNum, node->firstChild->firstChild->Valstr); 
            return NULL;
        }else if(right->kind!=BASIC||right->u.basic!=1){
            sprintError(12,node->firstChild->Sibc->Sibc->lineNum,node->firstChild->Sibc->Sibc->firstChild->nodeName); // value<->nodeName INT FLOAT
            return NULL;
        }else{
            Type res = (Type)malloc(sizeof(struct Type_));
			memcpy(res, left->u.array.elem, sizeof(struct Type_));
			res->assign = BOTH;
			return res;
        }
    }
}


FieldList Args(struct Node *node){
	if(node==NULL)	return NULL;
	Type t = Exp(node->firstChild);
    if(t==NULL){        return NULL; }
    FieldList f = (FieldList)malloc(sizeof(struct FieldList_));
	f->name = node->firstChild->Valstr; 
    f->type = t;
    if(node->firstChild->Sibc!=NULL){ 
        f->tail = Args(node->firstChild->Sibc->Sibc);
    }
    return f;
}

Type Type_get_f(FieldList f, char* name){
    while(f!=NULL){
        if(strcmp(f->name,name)==0){
            return f->type;
        }
        f = f->tail;
    }
    return NULL;
}

int Type_check(Type t1,Type t2){
    if(t1==NULL&&t2==NULL){ return 1;    }
    if(t1==NULL||t2==NULL||t1->kind!=t2->kind){ return 0;}
    if(t1->kind == BASIC){
        if(t1->u.basic == t2->u.basic){return 1;}
        else
            return 0;}
    else if(t1->kind == ARRAY){return Type_check(t1->u.array.elem, t2->u.array.elem);}
    else if(t1->kind == STRUCTURE){
        if(t2->kind!=STRUCTURE){
            return 0;
        }else{
            return Struct_check(t1->u.structure,t2->u.structure);
        }
    }
}

int Struct_check(Structure s1,Structure s2){
    FieldList sd = s1->domain;
    FieldList rd = s2->domain;
    while (sd!=NULL&&rd!=NULL)
    {
        if(Type_check(sd->type,rd->type)==0){
            return 0;
        }else{
            sd = sd->tail;
            rd = rd->tail;
        }
    }
    if(sd==NULL&&rd==NULL){
        return 1;
    }else{
        return 0;
    }
}

int Param_check(FieldList p1,FieldList p2){
    while (p1!=NULL && p2!=NULL)
    {
        if(Type_check(p1->type,p2->type)==0){
            return 0;
        }
        p1 = p1->tail;
        p2 = p2->tail;
    }
    if(p1==NULL&&p2==NULL){
        return 1;
    }else{
        return 0;
    }
}

void sprintError(int errorType,int lineNum, char* str){
    fprintf(stderr,"Error type %d at Line %d: ",errorType,lineNum);
    switch (errorType)
    {
    case 1:
        fprintf(stderr,"Undefined variable \"%s\"\n",str);
        break;
    case 2:
        fprintf(stderr,"Undefined function \"%s\"\n",str);
        break;
    case 3:
        fprintf(stderr,"Redefined variable \"%s\"\n",str);
        break;
    case 4:
        fprintf(stderr,"Redefined function \"%s\"\n",str);
        break;
    case 5:
        fprintf(stderr,"Type mismatched for assignment\n");
        break;
    case 6:
        fprintf(stderr,"The left-hand side of an assignment must be a variable.\n");
        break;
    case 7:
        fprintf(stderr,"Type mismatched for operands.\n");
        break;
    case 8:
        fprintf(stderr,"Type mismatched for return.\n");
        break;
    case 9:
        fprintf(stderr,"Function \"%s\" is not applicable for arguments.\n",str);
        break;
    case 10:
        fprintf(stderr,"Illegal use of of \"[]\" operator\n");
        break;
    case 11:
        fprintf(stderr,"Illegal use of of \"()\" operator\n");
        break;
    case 12:
        fprintf(stderr,"Array index is not an integer:%s\n", str);
        break;
    case 13:
        fprintf(stderr,"Illegal use of \".\".\n");
        break;
    case 14:
        fprintf(stderr,"Non-existent field: \"%s\".\n",str);
        break;
    case 15:
        fprintf(stderr,"Redefined field: \"%s\".\n",str);
        break;
    case 16:
        fprintf(stderr,"Duplicated name: \"%s\".\n",str);
        break;
    case 17:
        fprintf(stderr,"Undefined structure: \"%s\".\n",str);
        break;
    case 18:
        fprintf(stderr,"Undefined function:%s Only declared.\n", str);
        break;
    case 19:
        fprintf(stderr,"Inconsistent declearation of function:%s\n", str);
        break;    
    default:
        break;
    }

}

