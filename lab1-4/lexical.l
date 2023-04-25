%{
    #include "syntax.tab.h"
    int yycolumn = 1;
    #define YY_USER_ACTION \
        yylloc.first_line = yylloc.last_line = yylineno; \
	    yylloc.first_column = yycolumn; \
	    yylloc.last_column = yycolumn + yyleng - 1; \
	    yycolumn += yyleng;
    extern void printError(char errorType, int lineno, char* msg);
    extern int Err_new(int errorLineno);
    extern int errorflag;
    extern int o_atoi(char* s);
    extern int x_atoi(char* s);
%}
letter  [A-Za-z]
digit   [0-9]
INT     0|([1-9]{digit}*)|([0][0-7]+)|([0][xX][0-9a-fA-F]+)
sciFLOAT    ({digit}+\.{digit}*|{digit}*\.{digit}+)[eE][+-]?{digit}+
FLOAT   ({INT}[.]{digit}+)|{sciFLOAT}
ID      ([_A-Za-z])({letter}|{digit}|_)*
wID     [0-9]({letter}|{digit}|_)+
RELOP   ">"|"<"|">="|"<="|"=="|"!="
blank   [ \t\r]
EOF     <<EOF>>
%option yylineno
%%
{RELOP} {yylval.type_string = strdup(yytext);return RELOP; }
";"     {return SEMI; }
","     {return COMMA; }
"="     {return ASSIGNOP; }
"+"     {return PLUS; }
"-"     {return MINUS; }
"*"     {return STAR; }
"/"     {return DIV; }  
"&&"    {return AND; }
"||"    {return OR; }
"."     {return DOT; }
"!"     {return NOT; }
"("     {return LP; }
")"     {return RP; }
"["     {return LB; }
"]"     {return RB; }
"{"     {return LC; }
"}"     {return RC; }
{INT}   {   if(yytext[0]=='0'){
                if(yytext[1]=='x'||yytext[1]=='X'){
                    yylval.type_int = x_atoi(yytext);
                } else {
                    yylval.type_int = o_atoi(yytext);
                };
            }else{
                yylval.type_int = atoi(yytext);
            };
            return INT;
        }
{FLOAT} {  
            yylval.type_float = atof(yytext);
            return FLOAT;
        }
"if"    {return IF; }
"else"  {return ELSE; }
"while" {return WHILE; }
"int"|"float"   {
                    yylval.type_string = strdup(yytext);
                    return TYPE; 
                }
"struct"    {return STRUCT; }
"return"    {return RETURN; }
"//"    { 
            char c = input();
            while (c != '\n') c = input();
        }
"/*" {
	char pre = input(), next = input();
  int flag = 0;
	while (next && !(pre == '*' && next == '/')) {
		  pre = next;
		  next = input();
	}
	if (!next) {
		printError('B', yylineno, "Unmatched \"/*\"");
	};
}
"*/"    {
                printError('B', yylineno, "Unmatched \"*/\"");
                errorflag = 1;
        }
{ID}    {
            yylval.type_string = strdup(yytext);
            return ID; 
        }
{wID}    {
            if (Err_new(yylineno)) {  printError('A', yylineno, yytext);  };
            yylval.type_string = strdup(yytext);
            return ID; 
            errorflag = 1;
            
        }
\n      { yycolumn = 1; }
{blank}    { }
.       { 
            if (Err_new(yylineno)) {
                char msg[32];
                sprintf(msg, "Mysterious character \"%s\"", yytext);
                printError('A', yylineno, msg);
                errorflag = 1;
            }
        }
%%
