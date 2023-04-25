#include "hashtable.h"
HashNode *gTable[CAPACITY];
HashNode *sTable[CAPACITY];
int hashFunc(char *key){
    unsigned int hash = 5381;
    while(*key){
        hash += (hash << 5 ) + (*key++);
    }
    return (hash & 0x7FFFFFFF) % CAPACITY;
}

void glbTable(){    //初始化表
	for(int i = 0;i<CAPACITY;i++){
		gTable[i]=NULL;
	}
	for(int i=0;i<CAPACITY;i++){
		sTable[i]=NULL;
	}
}

void strcTable(){
	for(int i=0;i<CAPACITY;i++){
		sTable[i]=NULL;
	}
}

void Hash_copy(HashNode *h1[],HashNode *h2[]){  //复制一张表
	for(int i=0;i<CAPACITY;i++){
		h2[i] = h1[i];
	}
}

Type Type_get(char *name){  //拿已经获取到的类型
    int hash = hashFunc(name);
    for(HashNode* node = gTable[hash];node!=NULL;node = node->next){
        if(strcmp(node->name, name)==0){ 
            return node->type;
        }
    }
    return NULL;
}

int scheck(char *name){ //检查s表中有无
    int hash = hashFunc(name);
    for(HashNode *node = sTable[hash];node!=NULL;node = node->next){
        if(strcmp(node->name,name)==0){ 
            return 1;
        }
    }
    return 0;
}

int check(char *name){ //检查大表中有无
    int hash = hashFunc(name);
    for(HashNode *node = gTable[hash];node!=NULL;node = node->next){
        if(strcmp(node->name,name)==0){
            return 1;
        }
    }
    return 0;
}

int checkfun(Function func){    //检查是否定义过函数，是否冲突
    char* name = func->name;
    int hash = hashFunc(name);
    for(HashNode *node = gTable[hash];node!=NULL;node = node->next){
        if(strcmp(node->name,name)==0 && node->type->kind==FUNCTION){ 
            if(Param_check(node->param, func->param)==0) return 2; //定义过但冲突
            return 1;
        }
    }
    return 0;
};

int checksf(char* name){    //检查函数名、结构名重复
    int hash = hashFunc(name);
    for(HashNode *node = gTable[hash];node!=NULL;node = node->next){
        if(strcmp(node->name,name)==0 && (node->type->kind==FUNCTION||node->type->kind==STRUCTURE )){ 
            return 1;
        }
    }
    return 0;
};

int check_new(char*name, int cdeep){    //检查嵌套中的语句
    for(int i = 0; i< al[cdeep]; i++){
        if(strcmp(a[cdeep][i]->name,name)==0) return 1;
    }
    return 0;
}

int sinsert(char *name,Type type){      //插入新表中
    int hash = hashFunc(name);
    HashNode *newnode = (HashNode *)malloc(sizeof(HashNode));
    newnode->name = name;
    newnode->type = type;
    HashNode *node = sTable[hash];
    if(node==NULL){        sTable[hash] = newnode;  }
    else{
        while(node->next!=NULL){
            node = node->next;
        }
        node->next = newnode;
    }
}

int insert(char *name,Type type){   //支持重复插入，返回值为插入的深度
    int hash = hashFunc(name);
    int k = 1;
    HashNode *newnode = (HashNode *)malloc(sizeof(HashNode));
    newnode->name = name;
    newnode->type = type;
    newnode->next = NULL;
    HashNode *node = gTable[hash];
    if(node==NULL){
        gTable[hash] = newnode;
        return 0;
    }else{
        while(node->next!=NULL){
            node = node->next;
            k++;
        }
        node->next = newnode;
        return k; 
    }
}

int insertFunc(Function func){  //插入函数，涉及到左右值
    HashNode *newnode = (HashNode *)malloc(sizeof(HashNode));
    newnode->name = func->name;
    newnode->type = (Type)malloc(sizeof(struct Type_));
    newnode->type->kind = FUNCTION;
    newnode->type->assign = RIGHT;
    newnode->type->u.function = func;
    newnode->param = func->param;
    int k = 1;
    int hash = hashFunc(func->name); 
    HashNode *node = gTable[hash];
    if(node==NULL){
        gTable[hash] = newnode;
        return 0;
    }else{
        while(node->next!=NULL){
            node = node->next;
            k++;
        }
        node->next = newnode;
        return k;
    }
}

void stackinsert(char* name,int CompStdeepth,int k){    //插入到栈中
    return ;
    struct hashstack* t = (struct hashstack*)malloc(sizeof(struct hashstack));
    t->name = name;
    t ->deep = k;
    a[CompStdeepth][al[CompStdeepth]]= t;
    al[CompStdeepth]++;
    return ;
}
void tabledelete(char* name, int deep){             //从哈希表上删除一个嵌套表
    return ;
    int hash = hashFunc(name);
    HashNode *node = gTable[hash];
    if(node->next==NULL){
        node =NULL;
        return;
    }
    while(deep > 1 ){
        node = node->next;
        deep--;
    }
    node->next = node->next->next;
    return ;
}
void stackclear(int compdeep){                         //清空嵌套表的栈
    return;
    for(int i = 0; i<al[compdeep];i++){
        tabledelete(a[compdeep][i]->name, a[compdeep][i]->deep);
    }
    for(int i = 0; i<al[compdeep];i++){
        a[compdeep][i]= NULL;
    }
    al[compdeep] = 0;
    return;
};