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
{int}        	{ yylval.type_int = atoi(yytext); return INT; }
{float}      	{ yylval.type_float = atof(yytext); return FLOAT; }
{char}			{ yylval.type_char = yytext[1]; return CHAR; }
"int"        	{ strcpy(yylval.type_id, yytext); return TYPE; }
"float"      	{ strcpy(yylval.type_id, yytext); return TYPE; }
"char"      	{ strcpy(yylval.type_id, yytext); return TYPE; }

"return"     	{ return RETURN; }
"if"         	{ return IF; }
"else"       	{ return ELSE; }
"while"      	{ return WHILE; }
"continue"      { return CONTINUE; }
"break"      	{ return BREAK; }
"for"			{ return FOR; } 

{id}       	 	{ strcpy(yylval.type_id, yytext); return ID; }
";"			 	{ return SEMI; }
","			 	{ return COMMA; }
">"|"<"|">="|"<="|"=="|"!=" { strcpy(yylval.type_id, yytext); return RELOP; }
"="			 	{ return ASSIGNOP; }
"+"			 	{ return PLUS; }
"-"			 	{ return MINUS; }
"*"			 	{ return STAR; }
"/"			 	{ return DIV; }
"&&"         	{ return AND; }
"++"			{ return INC; }
"--"			{ return DEC; }
"+="			{ return ADD_ASSIGN; }
"-="			{ return SUB_ASSIGN; }
"||"         	{ return OR; }
"!"			 	{ return NOT; }
"("			 	{ return LP; }
")"			 	{ return RP; }
"{"			 	{ return LC; }
"}"			 	{ return RC; }
"["				{ return LT; }
"]"				{ return RT; }

[\n]         	{ yycolumn = 1; }   
[ \r\t]      	{ }
"//"[^\n]*   	{ yycolumn = 1; }
"/*"([^\*]|(\*)*[^\*/])*(\*)*"*/" 	{ yycolumn = 1; } 
.			 	{ printf("Error type A :Mysterious character \"%s\"\n\t at Line %d\n",yytext,yylineno); }
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
