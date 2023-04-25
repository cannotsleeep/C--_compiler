%locations
%{
    #include <stdio.h>
    #include <string.h>
    #include <stdarg.h>
    #include <ctype.h>
    #include "lex.yy.c"
    #include "node.h"

    void yyerror(const char* s);
    int errorflag = 0;
    int last_error = 0;
    
    struct Node* Root = NULL;
    struct Node* constructNode(char* nodeName, enum NodeType nodeType, int lineNum);
    void construct(struct Node* fatherNode, int childNodeNum, ...);
    void Print_tree(struct Node* rootNode,int spaceNum);
    void printError(char errorType, int lineno, char* msg);
    int Err_new(int errorLineno);
%}
%union {
    int type_int;
    float type_float;
    char* type_string;
    struct Node* type_pnode;
}
%token <type_string> RELOP
%token ASSIGNOP
%token SEMI COMMA
%token PLUS MINUS STAR DIV AND OR NOT
%token DOT
%token LP RP LB RB LC RC
%token <type_int> INT
%token <type_float> FLOAT
%token <type_string> ID TYPE
%token IF ELSE WHILE STRUCT RETURN

%type <type_pnode> Program ExtDefList ExtDef ExtDecList Specifier StructSpecifier
%type <type_pnode> OptTag Tag VarDec FunDec VarList ParamDec CompSt
%type <type_pnode> StmtList Stmt DefList Def DecList Dec Exp Args

%nonassoc error
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left LP RP LB RB DOT
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
Program : ExtDefList {
            $$ = constructNode("Program", NTML, @$.first_line);
            construct($$, 1, $1);
            Root = $$;
        }
    | ExtDefList error {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Unexpected character");
                struct Node* nodeError = constructNode("error", NVL, @2.first_line);
                $$ = constructNode("Program", NTML, @$.first_line);
                construct($$, 2, $1, nodeError);
                Root = $$;
            } else {
            	$$ = NULL;
            }
        }
    ;
ExtDefList : ExtDef ExtDefList {
            $$ = constructNode("ExtDefList", NTML, @$.first_line);
            construct($$, 2, $1, $2);
        }
    |  {
            $$ = NULL;
        }
    ;
ExtDef : Specifier ExtDecList SEMI {
            $$ = constructNode("ExtDef", NTML, @$.first_line);
            construct($$, 3, $1, $2, constructNode("SEMI", NVL, @3.first_line));
        }
    | Specifier SEMI {
            struct Node* nodeSEMI = constructNode("SEMI", NVL, @2.first_line);
            $$ = constructNode("ExtDef", NTML, @$.first_line);
            construct($$, 2, $1, nodeSEMI);
        }
    | Specifier FunDec CompSt {
            $$ = constructNode("ExtDef", NTML, @$.first_line);
            construct($$, 3, $1, $2, $3);
        }
    | Specifier FunDec SEMI {
            struct Node* nodeSEMI = constructNode("SEMI", NVL, @3.first_line);
            $$ = constructNode("ExtDef", NTML, @$.first_line);
            construct($$, 3, $1, $2, nodeSEMI);
        }
    | Specifier error {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Missing \";\"");
                $$ = constructNode("ExtDef", NTML, @$.first_line);
                construct($$, 2, $1, constructNode("error", NVL, @2.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
ExtDecList : VarDec {
            $$ = constructNode("ExtDecList", NTML, @$.first_line);
            construct($$, 1, $1);
        }
    | VarDec COMMA ExtDecList {
            $$ = constructNode("ExtDecList", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("COMMA", NVL, @2.first_line), $3);
        }
    ;

Specifier : TYPE {
            struct Node* nodeTYPE = constructNode("TYPE", VL, @1.first_line);
            nodeTYPE->Valstr = $1;
            $$ = constructNode("Specifier", NTML, @$.first_line);
            construct($$, 1, nodeTYPE);
        }
    | StructSpecifier {
            $$ = constructNode("Specifier", NTML, @$.first_line);
            construct($$, 1, $1);
        }
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
            $$ = constructNode("StructSpecifier", NTML, @$.first_line);           
            construct($$, 5, constructNode("STRUCT", NVL, @1.first_line), $2, constructNode("LC", NVL, @3.first_line), $4, constructNode("RC", NVL, @5.first_line));
        }
    | STRUCT Tag {         
            $$ = constructNode("StructSpecifier", NTML, @$.first_line);
            construct($$, 2, constructNode("STRUCT", NVL, @1.first_line), $2);
        }
    | STRUCT OptTag LC DefList error {
            if (Err_new(@5.first_line)) {
                printError('B', @5.first_line, "Missing \"}\"");
                $$ = constructNode("StructSpecifier", NTML, @$.first_line);           
                construct($$, 5, constructNode("STRUCT", NVL, @1.first_line), $2, constructNode("LC", NVL, @3.first_line), $4, constructNode("error", NVL, @5.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
OptTag : ID {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("OptTag", NTML, @$.first_line);
            construct($$, 1, nodeID);
        }
    |   {
            $$ = NULL;
        }
    ;
Tag : ID {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("Tag", NTML, @$.first_line);
            construct($$, 1, nodeID);
        }
    ;

VarDec : ID {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("VarDec", NTML, @$.first_line);
            construct($$, 1, nodeID);
        }
    | VarDec LB INT RB {
            struct Node* nodeINT = constructNode("INT", VL, @3.first_line);
            nodeINT->Valint = $3;
            $$ = constructNode("VarDec", NTML, @$.first_line);
            construct($$, 4, $1, constructNode("LB", NVL, @2.first_line), nodeINT, constructNode("RB", NVL, @4.first_line));
        }
    | VarDec LB error RB {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error between \"[]\"");
                struct Node* nodeError = constructNode("error", NVL, @3.first_line);
                $$ = constructNode("VarDec", NTML, @$.first_line);
                construct($$, 4, $1, constructNode("LB", NVL, @2.first_line), nodeError, constructNode("RB", NVL, @4.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
FunDec : ID LP VarList RP {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("FunDec", NTML, @$.first_line);
            construct($$, 4, nodeID, constructNode("LP", NVL, @2.first_line), $3, constructNode("RP", NVL, @4.first_line));
        }
    | ID LP RP {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("FunDec", NTML, @$.first_line);
            construct($$, 3, nodeID, constructNode("LP", NVL, @2.first_line), constructNode("RP", NVL, @3.first_line));
        }
    | ID LP error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Missing \")\"");
                struct Node* nodeID = constructNode("ID", VL, @1.first_line);
                nodeID->Valstr = $1;
                $$ = constructNode("FunDec", NTML, @$.first_line);
                construct($$, 3, nodeID, constructNode("LP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            } 
        }
    | ID LP error RP {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error between ()");
                struct Node* nodeID = constructNode("ID", VL, @1.first_line);
                nodeID->Valstr = $1;
                $$ = constructNode("FunDec", NTML, @$.first_line);
                construct($$, 4, nodeID, constructNode("LP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line), constructNode("RP", NVL, @4.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | ID error RP {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Missing \"(\"");
                struct Node* nodeID = constructNode("ID", VL, @1.first_line);
                nodeID->Valstr = $1;
                $$ = constructNode("FunDec", NTML, @$.first_line);
                construct($$, 3, nodeID, constructNode("error", NVL, @2.first_line), constructNode("RP", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
VarList : ParamDec COMMA VarList {
            $$ = constructNode("VarList", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("COMMA", NVL, @2.first_line), $3);
        }
    | ParamDec {
            $$ = constructNode("VarList", NTML, @$.first_line);
            $$->firstChild = $1;
        }
    ;
ParamDec : Specifier VarDec {
            $$ = constructNode("ParamDec", NTML, @$.first_line);
            construct($$, 2, $1, $2);
        }
    ;

CompSt : LC DefList StmtList RC {
            $$ = constructNode("CompSt", NTML, @$.first_line);
            construct($$, 4, constructNode("LC", NVL, @1.first_line), $2, $3, constructNode("RC", NVL, @4.first_line));
        }
    | error DefList StmtList RC {
            if (Err_new(@1.first_line)) {
                printError('B', @1.first_line, "Missing \"{\"");
                $$ = constructNode("CompSt", NTML, @$.first_line);
                construct($$, 4, constructNode("error", NVL, @1.first_line), $2, $3, constructNode("LC", NVL, @4.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
StmtList : Stmt StmtList {
            $$ = constructNode("StmtList", NTML, @$.first_line);
            construct($$, 2, $1, $2);
        }
    | {
            $$ = NULL;
        }
    ;
Stmt : Exp SEMI {
            $$ = constructNode("Stmt", NTML, @$.first_line);
            construct($$, 2, $1, constructNode("SEMI", NVL, @2.first_line));
        }
    | CompSt {
            $$ = constructNode("Stmt", NTML, @$.first_line);
            $$->firstChild = $1;
        }
    | RETURN Exp SEMI {
            $$ = constructNode("Stmt", NTML, @$.first_line);
            construct($$, 3, constructNode("RETURN", NVL, @1.first_line), $2, constructNode("SEMI", NVL, @3.first_line));
        }
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE {
            $$ = constructNode("Stmt", NTML, @$.first_line);
            construct($$, 5, constructNode("IF", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), $3, constructNode("RP", NVL, @4.first_line), $5);
        }
    | IF LP Exp RP Stmt ELSE Stmt {
            $$ = constructNode("Stmt", NTML, @$.first_line);
            construct($$, 7, constructNode("IF", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), $3, constructNode("RP", NVL, @4.first_line), $5,  constructNode("ELSE", NVL, @6.first_line), $7);
        }
    | WHILE LP Exp RP Stmt {
            $$ = constructNode("Stmt", NTML, @$.first_line);
            construct($$, 5, constructNode("WHILE", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), $3, constructNode("RP", NVL, @4.first_line), $5);
        }
    | Exp error {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Missing \";\"");
                $$ = constructNode("Stmt", NTML, @$.first_line);                
                construct($$, 2, $1, constructNode("error", NVL, @2.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | RETURN Exp error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Missing \";\"");
                $$ = constructNode("Stmt", NTML, @$.first_line);                
                construct($$, 3, constructNode("RETURN", NVL, @1.first_line), $2, constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | error SEMI {
            if (Err_new(@1.first_line)) {
                printError('B', @1.first_line, "Syntax error in Exp1");
                $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 2, constructNode("error", NVL, @1.first_line), constructNode("SEMI", NVL, @2.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp2");
                $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 5, constructNode("IF", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line), constructNode("RP", NVL, @4.first_line), $5);
            } else {
            	$$ = NULL;
            }
        }
    | IF LP Exp error Stmt %prec LOWER_THAN_ELSE {
            if (Err_new(@4.first_line)) {
                printError('B', @4.first_line, "Missing \")\"");
               $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 5, constructNode("IF", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), $3, constructNode("error", NVL, @4.first_line), $5);
            } else {
            	$$ = NULL;
            }
        }
    | IF LP error RP Stmt ELSE Stmt {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp3");
                $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 7, constructNode("IF", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line), constructNode("RP", NVL, @4.first_line), $5, constructNode("ELSE", NVL, @6.first_line), $7);
            } else {
            	$$ = NULL;
            }
        }
    | IF LP Exp error Stmt ELSE Stmt {
            if (Err_new(@4.first_line)) {
                printError('B', @4.first_line, "Missing \")\"");
                $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 7, constructNode("IF", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), $3, constructNode("error", NVL, @4.first_line), $5, constructNode("ELSE", NVL, @6.first_line), $7);
            } else {
            	$$ = NULL;
            }
        }
    | WHILE LP error RP Stmt {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp4");
                $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 5, constructNode("WHILE", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line), constructNode("RP", NVL, @4.first_line), $5);
            } else {
            	$$ = NULL;
            }
        }
    | WHILE LP Exp error Stmt {
            if (Err_new(@4.first_line)) {
                printError('B', @4.first_line, "Missing \")\"");
                $$ = constructNode("Stmt", NTML, @$.first_line);
                construct($$, 5, constructNode("WHILE", NVL, @1.first_line), constructNode("LP", NVL, @2.first_line), $3, constructNode("error", NVL, @4.first_line), $5);
            } else {
            	$$ = NULL;
            }
        }
    ;

DefList : Def DefList {
            $$ = constructNode("DefList", NTML, @$.first_line);
            construct($$, 2, $1, $2);
        }
    |   {
            $$ = NULL;
        }
    ;
Def : Specifier DecList SEMI {
            $$= constructNode("Def", NTML, @$.first_line);
            construct($$, 3, $1, $2, constructNode("SEMI", NVL, @3.first_line));
        }
    | Specifier error SEMI {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Syntax error in DecList");
                $$ = constructNode("Def", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("error", NVL, @2.first_line), constructNode("SEMI", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
DecList : Dec {
            $$ = constructNode("DecList", NTML, @$.first_line);
            $$->firstChild = $1;
        }
    | Dec COMMA DecList {
            $$ = constructNode("DecList", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("COMMA", NVL, @2.first_line), $3);
        }
    ;
Dec : VarDec {
            $$ = constructNode("Dec", NTML, @$.first_line);
            $$->firstChild = $1;
        }
    | VarDec ASSIGNOP Exp {
            $$ = constructNode("Dec", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("ASSIGNOP", NVL, @2.first_line), $3);
        }
    | VarDec ASSIGNOP error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp5");
                $$ = constructNode("Dec", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("ASSIGNOP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
Exp : Exp ASSIGNOP Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("ASSIGNOP", NVL, @2.first_line), $3);
        }
    | Exp AND Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("AND", NVL, @2.first_line), $3);
        }
    | Exp OR Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("OR", NVL, @2.first_line), $3);
        }
    | Exp RELOP Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            struct Node* relop = constructNode("RELOP", VL, @2.first_line);
            relop->Valstr = $2;
            construct($$, 3, $1, relop, $3);
            
        }
    | Exp PLUS Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("PLUS", NVL, @2.first_line), $3);
        }
    | Exp MINUS Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("MINUS", NVL, @2.first_line), $3);
        }
    | Exp STAR Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("STAR", NVL, @2.first_line), $3);
        }
    | Exp DIV Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("DIV", NVL, @2.first_line), $3);
        }
    | LP Exp RP {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, constructNode("LP", NVL, @1.first_line), $2, constructNode("RP", NVL, @3.first_line));
        }
    | MINUS Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 2, constructNode("MINUS", NVL, @1.first_line), $2);
        }
    | NOT Exp {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 2, constructNode("NOT", NVL, @1.first_line), $2);
        }
    | ID LP Args RP {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 4, nodeID, constructNode("LP", NVL, @2.first_line), $3, constructNode("RP", NVL, @4.first_line));
        }
    | ID LP RP {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, nodeID, constructNode("LP", NVL, @2.first_line), constructNode("RP", NVL, @3.first_line));
        }
    | Exp LB Exp RB {
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 4, $1, constructNode("LB", NVL, @2.first_line), $3, constructNode("RB", NVL, @4.first_line));
        }
    | Exp DOT ID {
            struct Node* nodeID = constructNode("ID", VL, @3.first_line);
            nodeID->Valstr = $3;
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("DOT", NVL, @2.first_line), nodeID);
        }
    | ID {
            struct Node* nodeID = constructNode("ID", VL, @1.first_line);
            nodeID->Valstr = $1;
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 1, nodeID);
        }
    | INT {
            struct Node* nodeINT = constructNode("INT", VL, @1.first_line);
            nodeINT->Valint = $1;
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 1, nodeINT);
        }
    | FLOAT {
            struct Node* nodeFLOAT = constructNode("FLOAT", VL, @1.first_line);
            nodeFLOAT->Valfloat = $1;
            $$ = constructNode("Exp", NTML, @$.first_line);
            construct($$, 1, nodeFLOAT);
        }
    | Exp LB error RB {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error between \"[]\"");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 4, $1, constructNode("LB", NVL, @2.first_line), constructNode("error", NVL, @3.first_line), constructNode("RB", NVL, @4.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | error RP {
            if (Err_new(@1.first_line)) {
                printError('B', @1.first_line, "Missing \"(\"");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 2, constructNode("error", NVL, @1.first_line), constructNode("RP", NVL, @2.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | ID LP Args error {
            if (Err_new(@4.first_line)) {
                printError('B', @4.first_line, "Missing \")\"");
                struct Node* nodeID = constructNode("ID", VL, @1.first_line);
                nodeID->Valstr = $1;
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 4, nodeID, constructNode("LP", NVL, @2.first_line), $3, constructNode("error", NVL, @4.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | ID LP error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Missing \")\"");
                struct Node* nodeID = constructNode("ID", VL, @1.first_line);
                nodeID->Valstr = $1;
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, nodeID, constructNode("LP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp ASSIGNOP error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp6");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("ASSIGNOP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp AND error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp7");            
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("AND", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp OR error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp8");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("OR", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp RELOP error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp9");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("RELOP", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp PLUS error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp10");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("PLUS", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp MINUS error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp11");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("MINUS", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp STAR error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp12");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("STAR", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | Exp DIV error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Syntax error in Exp13");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, $1, constructNode("DIV", NVL, @2.first_line), constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | MINUS error {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Syntax error in Exp14");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 2, constructNode("MINUS", NVL, @1.first_line), constructNode("error", NVL, @2.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | NOT error {
            if (Err_new(@2.first_line)) {
                printError('B', @2.first_line, "Syntax error in Exp15");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 2, constructNode("NOT", NVL, @1.first_line), constructNode("error", NVL, @2.first_line));
            } else {
            	$$ = NULL;
            }
        }
    | LP Exp error {
            if (Err_new(@3.first_line)) {
                printError('B', @3.first_line, "Missing \")\"");
                $$ = constructNode("Exp", NTML, @$.first_line);
                construct($$, 3, constructNode("LP", NVL, @1.first_line), $2, constructNode("error", NVL, @3.first_line));
            } else {
            	$$ = NULL;
            }
        }
    ;
Args : Exp COMMA Args {
            $$ = constructNode("Args", NTML, @$.first_line);
            construct($$, 3, $1, constructNode("COMMA", NVL, @2.first_line), $3);
        }
    | Exp {
            $$ = constructNode("Args", NTML, @$.first_line);
            construct($$, 1, $1);
        }
    ;

%%
struct Node* constructNode(char* Name, enum NodeType Type, int line) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->nodeName = Name;
    newNode->nodeType = Type;
    newNode->lineNum = line;
    newNode->firstChild = NULL;
    newNode->Sibc = NULL;
    return newNode;
}

void construct(struct Node* fatherNode, int childNodeNum, ...) {
    va_list valist;
    va_start(valist, childNodeNum);
    struct Node* firstChild = NULL;
    struct Node* lastChild = NULL;
    for (int i = 0; i < childNodeNum; i++) {
        struct Node* curNode = va_arg(valist, struct Node*);
        if (firstChild == NULL) {
            if (curNode != NULL) {
                firstChild = curNode;
                lastChild = firstChild;
            }
        } else {
            if (curNode != NULL) {
                lastChild->Sibc = curNode;
                lastChild = curNode;
            }
        }
    }
    va_end(valist);
    fatherNode->firstChild = firstChild;
}

void Print_tree(struct Node* rootNode, int spaceNum) {
    if (rootNode == NULL)
        return;
    for (int i = 0; i < spaceNum; i++) {
        printf(" ");
    }
    switch (rootNode->nodeType) {
        case NTML:
            printf("%s (%d)\n", rootNode->nodeName, rootNode->lineNum);
            break;
        case NVL:
            printf("%s\n", rootNode->nodeName);
            break;
        case VL:
            printf("%s: ", rootNode->nodeName);
            if ((strcmp(rootNode->nodeName, "TYPE") == 0) || (strcmp(rootNode->nodeName, "ID") == 0) || (strcmp(rootNode->nodeName, "RELOP") == 0)) {
                printf("%s\n", rootNode->Valstr);
            } else if (strcmp(rootNode->nodeName, "INT") == 0) {
                printf("%d\n", rootNode->Valint);
            } else if (strcmp(rootNode->nodeName, "FLOAT") == 0) {
                printf("%f\n", rootNode->Valfloat);
            } else {
                printf("ERROR!\n");
            }
            break;
        default:
            printf("ERROR!\n");
    }
    spaceNum += 2;
    struct Node* firstChild = rootNode->firstChild;
    if (firstChild != NULL) {
        Print_tree(firstChild, spaceNum);
        struct Node* sibling = firstChild->Sibc;
        while (sibling != NULL) {
            Print_tree(sibling, spaceNum);
            sibling = sibling->Sibc;
        }
    }
}

void yyerror(const char* s) { }

void printError(char errorType, int lineno, char* msg) {
    fprintf(stderr, "Error type %c at Line %d: %s.\n", errorType, lineno, msg);
    errorflag = 1;
}

int Err_new(int errorLineno) {
    if (last_error != errorLineno) {
        errorflag = 1;
        last_error = errorLineno;
        return 1;
    } else {
        return 0;
    }
}

void Pt_finish(struct Node* rootNode) {
    if (rootNode == NULL) {
        return;
    }
    struct Node* curNode = rootNode->firstChild;
    struct Node* nextNode = NULL;
    while (curNode != NULL) {
        nextNode = curNode->Sibc;
        Pt_finish(curNode);
        curNode = nextNode;
    }
    if ((strcmp(rootNode->nodeName, "TYPE") == 0) || (strcmp(rootNode->nodeName, "ID") == 0)) {
        free(rootNode->Valstr);
        rootNode->Valstr = NULL;
    }
    free(rootNode);
    rootNode = NULL;
};
int o_atoi(char *s)
{
	int t=strlen(s);
    int sum=0;
    int q;
    for(int i=1;i<t;i++){
        q=(int)s[i]-(int)'0';
    sum = (sum + q)*8;}
    sum = sum/8;
	return (int)sum;
}
int x_atoi(char *s)
{
	int t=strlen(s);
    long sum=0;
    int q;
    for(int i=2;i<t;i++){
    if(s[i]>='A' && s[i]<='Z')  q=(int)s[i]-(int)'A'+10;
    if(s[i]>='a' && s[i]<='z')  q=(int)s[i]-(int)'a'+10;
    if(s[i]>='0' && s[i]<='9')  q=(int)s[i]-(int)'0';
    sum = (sum + q)*16;}
    sum = sum/16;
	return (long)sum;
}