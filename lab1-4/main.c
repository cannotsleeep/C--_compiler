#include <stdio.h>
#include "IR.h"
#include "asm.h"
extern FILE* yyin;
extern int yyparse (void);
extern void yyrestart (FILE *input_file  );
extern int errorflag;
extern struct Node* Root;
extern void semantic(struct Node* rootNode);
extern void Pt_finish(struct Node* rootNode);
extern void Print_tree(struct Node* rootNode,int spaceNum);


int main(int argc, char** argv) {
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f) {   perror(argv[1]);   return 1; }
    yyrestart(f);
    yyparse();  //处理文件
    //if (!errorflag) {  Print_tree(Root,0);  }
    if (!errorflag) {  semantic(Root);  }
    //fprintf(stderr, "Nwrong!\n");
    CodeList codelisthead = Intercode(Root);
    FILE* ff;
    if(argv[2] == NULL){ff = fopen("output.asm", "w");}
        else {ff =fopen(argv[2], "w");};
    //print_IR(codelisthead, fx);
    print_asm(codelisthead, ff);
    fclose(ff);
    return 0;    
}
