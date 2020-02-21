%{
	#include "parser.tab.h"
	#include "string.h"
	#include "def.h"
	int yycolumn = 1;
	#define YY_USER_ACTION \
		yylloc.first_line = yylloc.last_line=yylineno; \
		yylloc.first_column = yycolumn; \
		yylloc.last_column = yycolumn+yyleng-1; \
		yycolumn += yyleng;
		
	typedef union 
	{
		int type_int;
		int type_float;
		int type_char;
		char type_id[32];
		struct node *ptr;
	} YYLVAL;
#define YYSTYPE YYLVAL

%}
%option yylineno

id   	[A-Za-z_][A-Za-z0-9_]*  
int    	[0-9]|[1-9][0-9]+
float  	([0-9]*\.[0-9]+)|([0-9]+\.)
char	'.'

%%
{int}        	{ yylval.type_int = atoi(yytext); printf("(intnum, %s)\n", yytext); return INT; }
{float}      	{ yylval.type_float = atof(yytext); printf("(flaotnum, %s)\n", yytext); return FLOAT; }
{char}			{ yylval.type_char = yytext[1]; printf("(char, %c)\n", yytext[1]); return CHAR; }
"int"        	{ strcpy(yylval.type_id, yytext); printf("(keyword, %s)\n", yytext); return TYPE; }
"float"      	{ strcpy(yylval.type_id, yytext); printf("(keyword, %s)\n", yytext); return TYPE; }
"char"      	{ strcpy(yylval.type_id, yytext); printf("(keyword, %s)\n", yytext); return TYPE; }

"return"     	{ printf("(keyword, %s)\n", yytext); return RETURN; }
"if"         	{ printf("(keyword, %s)\n", yytext); return IF; }
"else"       	{ printf("(keyword, %s)\n", yytext); return ELSE; }
"while"      	{ printf("(keyword, %s)\n", yytext); return WHILE; }
"continue"      { printf("(keyword, %s)\n", yytext); return CONTINUE; }
"break"      	{ printf("(keyword, %s)\n", yytext); return BREAK; }
"for"			{ printf("(keyword, %s)\n", yytext); return FOR; } 

{id}       	 	{ strcpy(yylval.type_id, yytext); printf("(identifier, %s)\n", yytext); return ID; }
";"			 	{ printf("(punctuator, %s)\n", yytext); return SEMI; }
","			 	{ printf("(punctuator, %s)\n", yytext); return COMMA; }
">"|"<"|">="|"<="|"=="|"!=" { strcpy(yylval.type_id, yytext); printf("(operator, %s)\n", yytext); return RELOP; }
"="			 	{ printf("(operator, %s)\n", yytext); return ASSIGNOP; }
"+"			 	{ printf("(operator, %s)\n", yytext); return PLUS; }
"-"			 	{ printf("(operator, %s)\n", yytext); return MINUS; }
"*"			 	{ printf("(operator, %s)\n", yytext); return STAR; }
"/"			 	{ printf("(operator, %s)\n", yytext); return DIV; }
"&&"         	{ printf("(operator, %s)\n", yytext); return AND; }
"++"			{ printf("(operator, %s)\n", yytext); return INC; }
"--"			{ printf("(operator, %s)\n", yytext); return DEC; }
"+="			{ printf("(operator, %s)\n", yytext); return ADD_ASSIGN; }
"-="			{ printf("(operator, %s)\n", yytext); return SUB_ASSIGN; }
"||"         	{ printf("(operator, %s)\n", yytext); return OR; }
"!"			 	{ printf("(operator, %s)\n", yytext); return NOT; }
"("			 	{ printf("(punctuator, %s)\n", yytext); return LP; }
")"			 	{ printf("(punctuator, %s)\n", yytext); return RP; }
"{"			 	{ printf("(punctuator, %s)\n", yytext); return LC; }
"}"			 	{ printf("(punctuator, %s)\n", yytext); return RC; }
"["				{ printf("(punctuator, %s)\n", yytext); return LT; }
"]"				{ printf("(punctuator, %s)\n", yytext); return RT; }

[\n]         	{ yycolumn = 1; }   
[ \r\t]      	{ }
"//"[^\n]*   	{ yycolumn = 1; }
"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/" 	{ yycolumn = 1; } 
.			 	{ printf("Error type A :Mysterious character \"%s\"\n\t at Line %d\n", yytext, yylineno); }
%%

/* 和bison联用时，不需要这部分
void main()
{
yylex();
return 0;
}
*/

int yywrap()
{
	return 1;
}
