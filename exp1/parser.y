%error-verbose
%locations
%{
	#include "stdio.h"
	#include "math.h"
	#include "string.h"
	#include "def.h"
	extern int yylineno;
	extern char *yytext;
	extern FILE *yyin;
	char *filename;
	void yyerror(const char* fmt, ...);
	void display(struct ASTNode *, int);
%}

%union 
{
	int    type_int;
	float  type_float;
	char   type_char;
	char   type_id[32];
	struct ASTNode *ptr;
};

//  %type 定义非终结符的语义值类型
%type <ptr> program ExtDefList ExtDef Specifier ExtDecList FuncDec CompSt 
%type <ptr> VarList VarDec ParamDec Stmt StmList DefList Def DecList Dec Exp Args 
%type <ptr> ArrayDec
//CaseStmtList0 CaseStmtList
//% token 定义终结符的语义值类型
%token <type_int> 	INT              		 /*指定INT的语义值是type_int，有词法分析得到的数值*/
%token <type_id> 	ID RELOP TYPE    		 /*指定ID, RELOP 的语义值是type_id，有词法分析得到的标识符字符串*/
%token <type_char>	CHAR
%token <type_float> FLOAT          	   		 /*指定ID的语义值是type_id，有词法分析得到的标识符字符串*/

%token DPLUS LP RP LC RC LT RT SEMI COMMA    /*用bison对该文件编译时，带参数-d，生成的.tab.h中给这些单词进行编码，可在lex.l中包含parser.tab.h使用这些单词种类码*/
%token PLUS MINUS STAR DIV ASSIGNOP AND OR NOT IF ELSE WHILE RETURN FOR SWITCH CASE COLON DEFAULT CONTINUE BREAK
%token INC DEC ADD_ASSIGN SUB_ASSIGN
/*以下为接在上述token后依次编码的枚举常量，作为AST结点类型标记*/
%token PROGRAM EXT_DEF_LIST EXT_VAR_DEF FUNC_DEF FUNC_DEC EXT_DEC_LIST PARAM_LIST PARAM_DEC 
%token VAR_DEF DEC_LIST DEF_LIST COMP_STM STM_LIST EXP_STMT IF_THEN IF_THEN_ELSE
%token FUNC_CALL ARGS FUNCTION PARAM ARG CALL LABEL GOTO JLT JLE JGT JGE EQ NEQ
%token ARRAY_DEC ARRAY_VIS ARRAY_DEF ARRAY_GROUP

%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%left ADD_ASSIGN SUB_ASSIGN
%left INC DEC
%right UMINUS NOT DPLUS

%nonassoc LOWER_THEN_ELSE
%nonassoc ELSE
%%

program			//声明全局变量及函数等
	: ExtDefList    				{ $$ = mknode(1, PROGRAM, yylineno, $1); strcpy($$->type_id, filename); display($$, 0); }     //显示语法树,语义分析
    ; 
	
ExtDefList		//变量、函数列表
	: 						  		{ $$ = NULL; }
    | ExtDef ExtDefList 		  	{ $$ = mknode(2, EXT_DEF_LIST, yylineno, $1, $2); }   //每一个EXTDEFLIST的结点，其第1棵子树对应一个外部变量声明或函数
    ; 
	
ExtDef			//声明变量、函数
	: Specifier ExtDecList SEMI   	{ $$ = mknode(2, EXT_VAR_DEF, yylineno, $1, $2); }   //该结点对应一个外部变量声明
    | Specifier FuncDec CompSt    	{ $$ = mknode(3, FUNC_DEF, yylineno, $1, $2, $3); }         //该结点对应一个函数定义
    | error SEMI   			  		{ $$ = NULL; }
    ;
	
Specifier		//关键字类型
	: TYPE   					  	{ $$ = mknode(0, TYPE, yylineno); strcpy($$->type_id, $1); $$->type = !strcmp($1, "int")? INT: !strcmp($1, "float")? FLOAT : CHAR; }   
    ;      
	
ExtDecList		//标识符列表
	: VarDec         			  		{ $$ = $1; }       /*每一个EXT_DECLIST的结点，其第一棵子树对应一个变量名(ID类型的结点),第二棵子树对应剩下的外部变量名*/
    | VarDec COMMA ExtDecList  		{ $$ = mknode(2, EXT_DEC_LIST, yylineno, $1, $3); }
    ;  
	
VarDec			//标识符
	: ID          				  	{ $$ = mknode(0, ID, yylineno); strcpy($$->type_id, $1); }   //ID结点，标识符符号串存放结点的type_id
	| ArrayDec						{ $$ = $1; }			
    ;
	
ArrayDec
	: ID LT INT RT 					{ $$ = mknode(0, ARRAY_DEC, yylineno); strcpy($$->type_id, $1); $$->array_size = $3; }
	;
	
FuncDec			//函数声明
	: ID LP VarList RP   		  	{ $$ = mknode(1, FUNC_DEC, yylineno, $3); strcpy($$->type_id, $1); }//函数名存放在$$->type_id
	| ID LP RP   				  	{ $$ = mknode(0, FUNC_DEC, yylineno); strcpy($$->type_id, $1); $$->ptr[0] = NULL; }//函数名存放在$$->type_id
    ;  
	
VarList			//变量
	: ParamDec  					{ $$ = mknode(1, PARAM_LIST, yylineno, $1); }
    | ParamDec COMMA VarList  	  	{ $$ = mknode(2, PARAM_LIST, yylineno, $1, $3); }
    ;
	
ParamDec		//参数声明
	: Specifier VarDec            	{ $$ = mknode(2, PARAM_DEC, yylineno, $1, $2); }
    ;
	
CompSt			//函数体定义
	: LC DefList StmList RC         { $$ = mknode(2, COMP_STM, yylineno, $2, $3); }
    ;
	
StmList			//语句列表
	: 							  	{ $$ = NULL; }  
    | Stmt StmList  			  	{ $$ = mknode(2, STM_LIST, yylineno, $1, $2); }
    ;
	
Stmt			//语句（表达式、复合语句、return语句、if语句、while语句等）
	: Exp SEMI    				  	{ $$ = mknode(1, EXP_STMT, yylineno, $1);}
    | CompSt      				  	{ $$ = $1; }      //复合语句结点直接最为语句结点，不再生成新的结点
    | RETURN Exp SEMI   			{ $$ = mknode(1, RETURN, yylineno, $2);}
    | IF LP Exp RP Stmt %prec LOWER_THEN_ELSE   		{ $$ = mknode(2, IF_THEN, yylineno, $3, $5); }
    | IF LP Exp RP Stmt ELSE Stmt   { $$ = mknode(3, IF_THEN_ELSE, yylineno, $3, $5, $7); }
    | WHILE LP Exp RP Stmt 		  	{ $$ = mknode(2, WHILE, yylineno, $3, $5); }
	| CONTINUE SEMI					{ $$ = mknode(0, CONTINUE, yylineno); strcpy($$->type_id, "CONTINUE"); }
	| BREAK	SEMI					{ $$ = mknode(0, BREAK, yylineno); strcpy($$->type_id, "BREAK"); }
    ;
	
DefList			//变量定义列表
	: 							  	{ $$ = NULL; }
    | Def DefList 				  	{ $$ = mknode(2, DEF_LIST, yylineno, $1, $2); }
    | error SEMI   				  	{ $$ = NULL; }
    ;
	
Def				//变量定义
	: Specifier DecList SEMI 		{ $$ = mknode(2, VAR_DEF, yylineno, $1, $2); }
    ;
	
DecList			//变量声明列表
	: Dec  						 	{ $$ = mknode(1, DEC_LIST, yylineno, $1); }
    | Dec COMMA DecList  		  	{ $$ = mknode(2, DEC_LIST, yylineno, $1, $3); }
	;
	
Dec				//变量声明
	: VarDec  					  	{ $$ = $1; }
    | VarDec ASSIGNOP Exp  		  	{ $$ = mknode(2, ASSIGNOP, yylineno, $1, $3); strcpy($$->type_id, "ASSIGNOP"); }
    ;
	
Exp				//表达式
	: Exp ASSIGNOP Exp 			  	{ $$ = mknode(2, ASSIGNOP, yylineno, $1, $3); strcpy($$->type_id, "ASSIGNOP"); }//$$结点type_id空置未用，正好存放运算符
    | Exp AND Exp   				{ $$ = mknode(2, AND, yylineno, $1, $3); strcpy($$->type_id, "AND"); }
    | Exp OR Exp    				{ $$ = mknode(2, OR, yylineno, $1, $3); strcpy($$->type_id, "OR"); }
    | Exp RELOP Exp 				{ $$ = mknode(2, RELOP, yylineno, $1, $3); strcpy($$->type_id, $2); }  //词法分析关系运算符号自身值保存在$2中
    | Exp PLUS Exp  			    { $$ = mknode(2, PLUS, yylineno, $1, $3); strcpy($$->type_id, "PLUS"); }
    | Exp MINUS Exp 				{ $$ = mknode(2, MINUS, yylineno, $1, $3); strcpy($$->type_id, "MINUS"); }
    | Exp STAR Exp  				{ $$ = mknode(2, STAR, yylineno, $1, $3); strcpy($$->type_id, "STAR"); }
    | Exp DIV Exp   				{ $$ = mknode(2, DIV, yylineno, $1, $3); strcpy($$->type_id, "DIV"); }	
	| INC Exp						{ $$ = mknode(1, INC, yylineno, $2); strcpy($$->type_id, "INC_LEFT"); }
	| DEC Exp						{ $$ = mknode(1, DEC, yylineno, $2); strcpy($$->type_id, "DEC_LEFT"); }	
	| Exp INC						{ $$ = mknode(1, INC, yylineno, $1); strcpy($$->type_id, "INC_RIGHT"); }
	| Exp DEC						{ $$ = mknode(1, DEC, yylineno, $1); strcpy($$->type_id, "DEC_RIGHT"); }	
	| Exp ADD_ASSIGN Exp			{ $$ = mknode(2, ADD_ASSIGN, yylineno, $1, $3); strcpy($$->type_id, "ADD_ASSIGN"); }
	| Exp SUB_ASSIGN Exp			{ $$ = mknode(2, SUB_ASSIGN, yylineno, $1, $3); strcpy($$->type_id, "SUB_ASSIGN"); }
    | LP Exp RP     				{ $$ = $2; }
    | MINUS Exp %prec UMINUS   	  	{ $$ = mknode(1, UMINUS, yylineno, $2); strcpy($$->type_id, "UMINUS"); }
    | NOT Exp       				{ $$ = mknode(1, NOT, yylineno, $2); strcpy($$->type_id, "NOT"); }
    | DPLUS Exp      			  	{ $$ = mknode(1, DPLUS, yylineno, $2); strcpy($$->type_id, "DPLUS"); }
    | Exp DPLUS      			  	{ $$ = mknode(1, DPLUS, yylineno, $1); strcpy($$->type_id, "DPLUS"); }
    | ID LP Args RP 				{ $$ = mknode(1, FUNC_CALL, yylineno, $3); strcpy($$->type_id, $1); }
    | ID LP RP      				{ $$ = mknode(0, FUNC_CALL, yylineno); strcpy($$->type_id, $1); }
    | ID            				{ $$ = mknode(0, ID, yylineno); strcpy($$->type_id, $1); }
    | INT           				{ $$ = mknode(0, INT, yylineno); $$->type_int = $1; $$->type = INT; }
    | FLOAT         				{ $$ = mknode(0, FLOAT, yylineno); $$->type_float = $1; $$->type = FLOAT; }
	| CHAR							{ $$ = mknode(0, CHAR, yylineno); $$->type_char = $1; $$->type = CHAR; }
	| ID LT Exp RT					{ $$ = mknode(1, ARRAY_VIS, yylineno, $3); strcpy($$->type_id, $1); }
	//| LC ArrGroup RC				{ $$ = mknode(1, ARRAY_DEF, yylineno, $2); strcpy($$->type_id, "ARR_ASSIGNOP"); }
    ;
	
Args		//调用函数参数定义
	: Exp COMMA Args    			{ $$ = mknode(2, ARGS, yylineno, $1, $3); }
    | Exp               			{ $$ = mknode(1, ARGS, yylineno, $1); }
    ;
/*
ArrGroup
	:								{ $$ = NULL; }
	| INT  							{ $$ = mknode(0, INT, yylineno); $$->type_int = $1; $$->type = INT; }
	| FLOAT 						{ $$ = mknode(0, FLOAT, yylineno); $$->type_float = $1; $$->type = FLOAT; } 							
	| CHAR 							{ $$ = mknode(0, CHAR, yylineno); $$->type_char = $1; $$->type = CHAR; }	
	| INT COMMA ArrGroup 			{ $$ = mknode(2, ARRAY_GROUP, yylineno, $1, $3); } 					
	| FLOAT COMMA ArrGroup 			{ $$ = mknode(2, ARRAY_GROUP, yylineno, $1, $3); } 
	| CHAR COMMA ArrGroup 			{ $$ = mknode(2, ARRAY_GROUP, yylineno, $1, $3); } 
*/      
%%

int main(int argc, char *argv[])
{
	yyin = fopen(argv[1], "r");
	if (!yyin) 
		return;
	filename = argv[1];
	yylineno = 1;
	yyparse();
	return 0;
}

#include<stdarg.h>
void yyerror(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Grammar Error at Line %d Column %d: ", yylloc.first_line, yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}
