#include "def.h"

/*
ERROR TYPES:
    Error type 1: undefined varible
    Error type 2: undefined function
    Error type 3: redefined varible
    Error type 4: redefined function
    Error type 5: Type mismatched for assignment
    Error type 6: The left-hand side of an assignment must be a varible
    Error type 7: Type mismatched for operands
    Error type 8: Type mismatched for return
    Error type 9: function call with too few arguments
    Error type 10: function call with too many arguments
    Error type 11: Mismatched arguments for function
    Error type 12: fibo is a function, not a varible
    Error type 13: f is not a function
    Error type 14: The length of an array must above zero
    Error type 15: Array index must be an integer
    Error type 16: The type of a bool expression must be integer
    Error type 17: a is not an array
    Error type 18: Index out of bound for array
    Error type 19: 'break' or 'continue' must be in a loop
*/

int err_num = 0;            //统计错误个数
int LEV = 0;                //层号
int func_size;              //1个函数的活动记录大小
int start = 0, end = 0;     //标记循环的开始与结束   

//拼接字符串s1和s2
char * strcat0(char *s1, char *s2)
{
    static char result[10];
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

//创建别名
char * newAlias() 
{
    static int no = 1;
    char s[10];
    itoa(no++, s, 10);
    return strcat0("v", s);
}

//创建临时变量名
char * newTemp()
{
    static int no = 1;
    char s[10];
    itoa(no++, s, 10);
    return strcat0("t", s);
}


int get_type(char *typename)
{
    if (!strcmp(typename, "int"))
        return INT;
    else if (!strcmp(typename, "float"))
        return FLOAT;
    else if (!strcmp(typename, "char"))
        return CHAR;
    else
        return -1;
}

int check_type(int type1, int type2)
{
    if((type1 == INT || type1 == FLOAT || type1 == CHAR) && 
        (type2 == INT || type2 == FLOAT || type2 == CHAR) && (type1 != type2))
        return 0;          
    else
        return 1;
}

char * newLabel() 
{
    static int no = 1;
    char s[10];
    itoa(no++, s, 10);
    return strcat0("label", s);
}


int get_width(int type)
{
    if (type == INT)
        return 4;
    else if (type == FLOAT)
        return 4;
    else if (type == CHAR)
        return 1;
    else
        return -1;
}

//生成一条TAC代码的结点组成的双向循环链表
CODE_NODE * genIR(int op, OPN opn1, OPN opn2, OPN result)
{
    CODE_NODE *head = (CODE_NODE *)malloc(sizeof(CODE_NODE));
    head->op = op;
    head->opn1 = opn1;
    head->opn2 = opn2;
    head->result = result;
    head->next = head->prior = head;
    return head;
}

//生成一条标号语句
CODE_NODE * genLabel(char *label)
{
    CODE_NODE * head = (CODE_NODE *)malloc(sizeof(CODE_NODE));
    head->op = LABEL;
    strcpy(head->result.id, label);
    head->next = head->prior = head;
    return head;
}

//生成GOTO语句
CODE_NODE * genGoto(char *label)
{
    CODE_NODE * head = (CODE_NODE *)malloc(sizeof(CODE_NODE));
    head->op = GOTO;
    strcpy(head->result.id, label);
    head->next = head->prior = head;
    return head;
}

//合并多个中间代码的双向循环链表，首尾相连
CODE_NODE * mergeCode(int num, ...)
{
    CODE_NODE *h1, *h2, *t1, *t2;
    va_list ap;
    va_start(ap, num);
    h1 = va_arg(ap, CODE_NODE *);
    while (--num > 0)
    {
        h2 = va_arg(ap, CODE_NODE *);
        if (h1 == NULL)
            h1 = h2;
        else if (h2)
        {
            t1 = h1->prior;
            t2 = h2->prior;
            t1->next = h2;
            t2->next = h1;
            h1->prior = t2;
            h2->prior = t1;
        }      
    }
    va_end(ap);
    return h1;
}

//输出中间代码
void printIR(CODE_NODE *head)
{
    char opnstr1[32], opnstr2[32], resultstr[32];
    CODE_NODE *h = head;
    do {
        if (h->opn1.kind == INT)
            sprintf(opnstr1, "#%d", h->opn1.const_int);
        if (h->opn1.kind == FLOAT)
            sprintf(opnstr1, "#%f", h->opn1.const_float);
        if (h->opn1.kind == CHAR)
            sprintf(opnstr1, "#%c", h->opn1.const_char);
        if (h->opn1.kind == ID)
            sprintf(opnstr1, "%s", h->opn1.id);

        if (h->opn2.kind == INT)
            sprintf(opnstr2, "#%d", h->opn2.const_int);
        if (h->opn2.kind == FLOAT)
            sprintf(opnstr2, "#%f", h->opn2.const_float);
        if (h->opn2.kind == CHAR)
            sprintf(opnstr2, "#%c", h->opn2.const_char);
        if (h->opn2.kind == ID)
            sprintf(opnstr2, "%s", h->opn2.id);
        sprintf(resultstr, "%s", h->result.id);
		
        switch (h->op)
		{
        case ASSIGNOP:  
			printf("%s := %s\n", resultstr, opnstr1);
            break;
				
        case PLUS:
        case MINUS:
        case STAR:
		case DIV: 
			printf("%s := %s %c %s\n", resultstr, opnstr1, 
                h->op == PLUS? '+' : h->op == MINUS? '-' : h->op==STAR? '*' : '/', opnstr2);
            break;

        case FUNCTION: 
			printf("\nFUNCTION %s :\n", h->result.id);
            break;
				
        case PARAM:    
			printf("PARAM %s\n", h->result.id);
            break;
			
        case LABEL:    
			printf("LABEL %s :\n", h->result.id);
            break;
			
        case GOTO:     
			printf("GOTO %s\n", h->result.id);
            break;
        
		case JLE:      
			printf("IF %s <= %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
		
        case JLT:      
			printf("IF %s < %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
			
        case JGE:      
			printf("IF %s >= %s GOTO %s\n", opnstr1 ,opnstr2, resultstr);
            break;
			
        case JGT:      
			printf("IF %s > %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
			
        case EQ:       
			printf("IF %s == %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
			
        case NEQ:      
			printf("IF %s != %s GOTO %s\n", opnstr1, opnstr2, resultstr);
            break;
			
        case ARG:      
			printf("ARG %s\n", h->result.id);
            break;
			
        case CALL:     
			if (!strcmp(opnstr1, "write"))
                printf("WRITE %s\n", resultstr);
            else if (!strcmp(opnstr1, "read"))
                printf("READ %s\n", resultstr);
            else
                printf("%s := CALL %s\n", resultstr, opnstr1);
            break;
			
        case RETURN:   
			if (h->result.kind)
                printf("RETURN %s\n", resultstr);
            else
                printf("RETURN\n");
            break;
        
        case ARRAY_DEC:
            printf("DEC %s %d\n", resultstr, h->opn1.const_int);
			break;
        }
		
    h = h->next;
    } while (h != head);
}

//输出语法错误
void semantic_error(int type, int line, char *msg1, char* msg2)
{
	err_num++;
    //这里可以只收集错误信息，最后一次显示
    printf("Error type %d at line %d: %s %s\n", type, line, msg1, msg2);
}

//打印符号表中的符号
void print_symbols()
{ 	
	//显示符号表
    int i = 0;
	printf("%s\t%s\t%s\t%s\t%s\t%s\n", "变量名", "别名", "层号", "类型", "标记", "偏移量");
    for(i = 0; i < symbolTable.index; i++)
        printf("%s\t%s\t%d\t%s\t%c\t%d\n", 
			symbolTable.symbols[i].name,
            symbolTable.symbols[i].alias, 
			symbolTable.symbols[i].level,
			symbolTable.symbols[i].type == INT? "int" : symbolTable.symbols[i].type == FLOAT? "float" : "char",
			symbolTable.symbols[i].flag, 
			symbolTable.symbols[i].offset);	
}

//查找符号表中变量或函数，找到后返回该元素索引，否则返回-1
int searchSymbolTable(char *name) 
{
    int i;
    for(i = symbolTable.index - 1; i >= 0; i--)
        if (!strcmp(symbolTable.symbols[i].name, name))  
			return i;
    return -1;
}

//向符号表插入一条记录
int fillSymbolTable(char *name, char *alias, int level, int type, char flag, int offset)
{
    //首先根据name查符号表，不能重复定义 重复定义返回-1
    int i;
    /*符号查重，考虑外部变量声明前有函数定义，
    其形参名还在符号表中，这时的外部变量与前函数的形参重名是允许的*/
    for(i = symbolTable.index - 1; i >= 0 && (symbolTable.symbols[i].level == level || level == 0); i--) 
	{
        if (level == 0 && symbolTable.symbols[i].level == 1)
			continue;  //外部变量和形参不必比较重名
        if (!strcmp(symbolTable.symbols[i].name, name))  
			return -1;
    }
    //填写符号表内容
    strcpy(symbolTable.symbols[symbolTable.index].name, name);
    strcpy(symbolTable.symbols[symbolTable.index].alias, alias);
    symbolTable.symbols[symbolTable.index].level = level;
    symbolTable.symbols[symbolTable.index].type = type;
    symbolTable.symbols[symbolTable.index].flag = flag;
    symbolTable.symbols[symbolTable.index].offset = offset;
    return symbolTable.index++; //返回的是符号在符号表中的位置序号，中间代码生成时可用序号取到符号别名
}


//填写临时变量到符号表，返回临时变量在符号表中的位置
int fill_Temp(char *name, int level, int type, char flag, int offset) 
{
    strcpy(symbolTable.symbols[symbolTable.index].name, "");
    strcpy(symbolTable.symbols[symbolTable.index].alias, name);
    symbolTable.symbols[symbolTable.index].level = level;
    symbolTable.symbols[symbolTable.index].type = type;
    symbolTable.symbols[symbolTable.index].flag = flag;
    symbolTable.symbols[symbolTable.index].offset = offset;
    return symbolTable.index++; //返回的是临时变量在符号表中的位置序号
}

//处理变量列表
void ext_var_list(struct ASTNode *T)
{	
    OPN opn1, opn2, result;
	if(T == NULL)
		return;

	int rtn, num;
	switch(T->kind)
	{	
	case EXT_DEC_LIST:     //VarDec COMMA ExtDecList
		T->ptr[0]->type = T->type;					//将类型属性向下传递变量结点
		T->ptr[0]->offset = T->offset;    			//外部变量的偏移量向下传递
        ext_var_list(T->ptr[0]);
        T->code = NULL;
		T->ptr[1]->type = T->type;             		//将类型属性向下传递变量结点
        T->ptr[1]->offset = T->offset + T->ptr[0]->width; 	//外部变量的偏移量向下传递
        //T->ptr[1]->width = T->width;	
        ext_var_list(T->ptr[1]);
        T->code = mergeCode(2, T->code, T->ptr[1]->code);
        T->num = T->ptr[1]->num + 1;				//统计参数个数
        break;
		
	case ID:
		rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'V', T->offset);  //最后一个变量名
		if (rtn == -1)
			semantic_error(3, T->pos, "Redefined varible", T->type_id);
		else 
			T->place = rtn;
		T->num = 1;
        T->width = get_width(T->type);
		break;
	
    case ARRAY_DEC:
        rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'V', T->offset);
        if (rtn == -1)
        {
            semantic_error(3, T->pos, "Redefined varible", T->type_id);
            break;
        }
        if (T->array_size == 0)
        {
            semantic_error(14, T->pos, "The length of an array must above zero", "");
            break;
        } 
        T->place = rtn;
        symbolTable.symbols[rtn].paramnum = T->array_size;
        T->width = get_width(T->type) * T->array_size;
        T->num = T->array_size;

        opn1.kind = INT;
        opn1.const_int = symbolTable.symbols[T->place].paramnum * get_width(T->type);

        result.kind = ID;
        strcpy(result.id, symbolTable.symbols[rtn].alias);
        T->code = genIR(ARRAY_DEC, opn1, opn2, result);
        break;

	default:
		break;
	}	
}

//函数参数匹配
//i表示函数在符号表中的索引
int match_param(int i, struct ASTNode *T)	
{	
	int num = symbolTable.symbols[i].paramnum;
	int pos = T->pos;
	int j, type1, type2;
	
	T = T->ptr[0];	
	if (num == 0 && T == NULL) 
		return 1;
	
	for (j = 1; j <= num; j++)
	{
		if (!T)
		{
            semantic_error(9, pos, "Function call with too few arguments", "");
            return 0;
        }
		//匹配参数类型
		type1 = symbolTable.symbols[i + j].type;	//形参类型
		type2 = T->ptr[0]->type;
		if (type1 != type2)
		{
            semantic_error(11, pos, "Mismatched arguments for function", T->type_id);
            return 0;
        }
		//继续匹配下一个参数
        T = T->ptr[1];
	}	
	
	if (T)
	{
		//num个参数已经匹配完，还有实参表达式
        semantic_error(10, pos, "Function call with too many arguments", "");
        return 0;
	}	
}

void translate_boolExp(struct ASTNode *T)
{
    OPN opn1, opn2, result;
    int op;
    int width, rtn;
    struct ASTNode * T0;
    if (T)
    {
        switch (T->kind)
        {
        case INT:
            if (T->type_int != 0)
                T->code = genGoto(T->Etrue);
            else
                T->code = genGoto(T->Efalse);           
            T->width = get_width(INT);
            break;
        
        case FLOAT:
            semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
            break;

        case CHAR:
            semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
            break;

        case ID:
            T->place = searchSymbolTable(T->type_id);
            if (rtn == -1)
            {
                semantic_error(1, T->pos, "undefined varible", T->type_id);
                break;
            }
            if (symbolTable.symbols[T->place].flag == 'F')
            {
                semantic_error(12, T->pos, T->type_id, "is a function, not a varible");
                break;
            }
            if (symbolTable.symbols[T->place].type != INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
                break;
            }
            
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->place].alias);
            opn1.offset = symbolTable.symbols[T->place].offset;

            opn2.kind = INT;
            opn2.const_int = 0;

            result.kind = ID;
            strcpy(result.id, T->Etrue);
            
            T->code = genIR(NEQ, opn1, opn2, result);
            T->code = mergeCode(2, T->code, genGoto(T->Efalse));
            break;

        case RELOP:
            T->ptr[0]->offset = T->offset;
           // printf("youwenti");
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            Exp(T->ptr[1]);
            if (T->width < T->ptr[1]->width)
                T->width = T->ptr[1]->width;

            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;

            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
            opn2.offset = symbolTable.symbols[T->ptr[1]->place].offset;

            result.kind = ID;
            strcpy(result.id, T->Etrue);

            if (strcmp(T->type_id, "<") == 0)           
                op = JLT;
            else if (strcmp(T->type_id, "<=") == 0)
                op = JLE;
            else if (strcmp(T->type_id, ">") == 0)
                op = JGT;
            else if (strcmp(T->type_id, ">=") == 0)
                op = JGE;
            else if (strcmp(T->type_id, "==") == 0)
                op = EQ;
            else if (strcmp(T->type_id, "!=") == 0)
                op = NEQ;

            T->code = genIR(op, opn1, opn2, result);
            T->code = mergeCode(4, T->ptr[0]->code, T->ptr[1]->code, T->code, genGoto(T->Efalse));
            break;

        case AND:
            strcpy(T->ptr[0]->Etrue, newLabel());
            strcpy(T->ptr[1]->Etrue, T->Etrue);
            strcpy(T->ptr[0]->Efalse, T->Efalse);
            strcpy(T->ptr[1]->Efalse, T->Efalse);

            translate_boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            translate_boolExp(T->ptr[1]);
            if (T->ptr[1]->width > T->width)
                T->width = T->ptr[1]->width;
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;

            T->code = mergeCode(3, T->ptr[0]->code, genLabel(T->ptr[0]->Etrue), T->ptr[1]->code);
            break;

        case OR:
            strcpy(T->ptr[0]->Etrue, T->Etrue);
            strcpy(T->ptr[1]->Etrue, T->Etrue);
            strcpy(T->ptr[0]->Efalse, newLabel());
            strcpy(T->ptr[1]->Efalse, T->Efalse);
            translate_boolExp(T->ptr[0]);
            translate_boolExp(T->ptr[1]);
            if (T->ptr[1]->width > T->ptr[0]->width)
                T->width = T->ptr[1]->width;
            else
                T->width = T->ptr[0]->width;
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            T->code = mergeCode(3, T->ptr[0]->code, genLabel(T->ptr[0]->Efalse), T->ptr[1]->code);
            break;

        case NOT:
            strcpy(T->ptr[0]->Etrue, T->Efalse);
            strcpy(T->ptr[0]->Efalse, T->Etrue);
            translate_boolExp(T->ptr[0]);
            T->code = T->ptr[0]->code;
            break;
        
        case FUNC_CALL:
            rtn = searchSymbolTable(T->type_id);
            if (rtn == -1)
            {
                semantic_error(2, T->pos, "Undefined function", T->type_id);
                break;
            }
            if (symbolTable.symbols[rtn].flag != 'F')
            {
                semantic_error(13, T->pos, T->type_id, "is not a function");
                break;
            }

            T->type = symbolTable.symbols[rtn].type;
            //width存放函数返回值的单数字节数
            width = get_width(T->type);

            //处理函数参数
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]);     //处理所有实参表达式求值，及类型
                T->width = T->ptr[0]->width + width;    //累加上计算实参使用临时变量的单元数
                //T.code = T1.code;
                T->code = T->ptr[0]->code;              
            }
            else
            {
                T->width = width;
                T->code = NULL;     //没有参数
            }
            match_param(rtn, T);

            T0 = T->ptr[0];
            //处理write
            if (strcmp(T->type_id, "write") == 0)
            {
                opn1.kind = ID;
                strcpy(opn1.id, T->type_id);
                opn1.offset = rtn;

                result.kind = ID;
                strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);
                T->code = mergeCode(2, T->code, genIR(CALL, opn1, opn2, result));
                break;
            }

            while (T0 != NULL)
            {
                result.kind = ID;
                strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);
                //T.code = T. code || ARG T01.alias
                T->code = mergeCode(2, T->code, genIR(ARG, opn1, opn2, result));
                T0 = T0->ptr[1];
            }
            
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->width - width);
            
            //中间代码生成 T.code = T.code || ti = CALL 函数名           
            opn1.kind = ID;
            strcpy(opn1.id, T->type_id);
            opn1.offset = rtn;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.offset = symbolTable.symbols[T->place].offset;
            T->code = mergeCode(2, T->code, genIR(CALL, opn1, opn2, result));

            // T.code = T.code || if ti != 0 goto T.Etrue || goto T.Efalse
            opn2.kind = INT;
            opn2.const_int = 0;

            result.kind = ID;
            strcpy(result.id, T->Etrue);
            T->code = mergeCode(3, T->code, genIR(NEQ, opn1, opn2, result), genGoto(T->Efalse));          
            break;

        default:
            break;
        }
    }
    

}

//处理基本表达式
void Exp(struct ASTNode *T)
{
	int rtn, rtn2, num, width;
    struct ASTNode *T0;
    OPN opn1, opn2, result;
    int op;
    char label1[15], label2[15];

    if (T)
    {
        switch (T->kind)
        {
        case ID:
            //查符号表，获得符号表中的位置，类型送type
            rtn = searchSymbolTable(T->type_id);
            int num = symbolTable.symbols[rtn].paramnum;
            if (rtn == -1)
            {
                semantic_error(1, T->pos, "Undefined varible", T->type_id);
                break;
            }                           
            if (symbolTable.symbols[rtn].flag == 'F')       
            {
                semantic_error(12, T->pos, T->type_id, "is a function, not a varible");
                break;
            }                    
            if (num > 0)
            {
                semantic_error(7, T->pos, T->type_id, "is an array, use [index] to access an array");
                break;
            }
            else
            {
                T->place = rtn;     //结点保存变量在符号表中的位置
                T->code = NULL;     //标识符不需要生成TAC
                T->type = symbolTable.symbols[rtn].type;
                T->offset = symbolTable.symbols[rtn].offset;
                T->width = 0;       //未再使用新单元
            }
            break;

        case INT:
            //为整型常量生成一个临时变量
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            T->type = INT;

            //生成中间代码
            opn1.kind = INT;
            opn1.const_int = T->type_int;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.offset = symbolTable.symbols[T->place].offset;
            T->code = genIR(ASSIGNOP, opn1, opn2, result);
            T->width = 4;
            break;

        case FLOAT:
            //为浮点常量生成一个临时变量
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            T->type = FLOAT;

            //生成中间代码
            opn1.kind = FLOAT;
            opn1.const_float = T->type_float;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.offset = symbolTable.symbols[T->place].offset;
            T->code = genIR(ASSIGNOP, opn1, opn2, result);
            T->width = 4;
            break;

        case CHAR:
            //为字符常量生成一个临时变量
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            T->type = CHAR;

            //生成中间代码
            opn1.kind = CHAR;
            opn1.const_char = T->type_char;
            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.offset = symbolTable.symbols[T->place].offset;
            T->code = genIR(ASSIGNOP, opn1, opn2, result);      
            T->width = 1;
            break;

        case ASSIGNOP:  //Exp ASSIGNOP Exp
            if (!(T->ptr[0]->kind == ID || T->ptr[0]->kind == ARRAY_DEC || T->ptr[0]->kind == ARRAY_VIS))           
                semantic_error(6, T->pos, "The left-hand side of an assignment must be a varible", "");
            else
            {
                Exp(T->ptr[0]);
                //printf("%d\n", T->ptr[0]->kind);
                T->ptr[1]->offset = T->offset;
                Exp(T->ptr[1]);

                if(!check_type(T->ptr[0]->type, T->ptr[1]->type))
                {
                    //printf("%d   %d\n", T->ptr[0]->type, T->ptr[1]->type);
                    semantic_error(5, T->pos, "Type mismatched for assignment", "");
                    break;
                }
                T->type = T->ptr[0]->type;      //树节点类型为“=”左边表达式类型
                T->width = T->ptr[1]->width;    //树节点的宽度为“=”号右边表达式宽度
                

                if (T->ptr[1]->kind == FUNC_CALL && strcmp(T->ptr[1]->type_id, "read") == 0)
                {
                    opn1.kind = ID;
                    strcpy(opn1.id, "read");
                    opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset;

                    result.kind = ID;
                    strcpy(result.id, symbolTable.symbols[T->ptr[1]->place].alias);
                    T->code = genIR(CALL, opn1, opn2, result);
                }
                
                opn1.kind = ID;
                strcpy(opn1.id, symbolTable.symbols[T->ptr[1]->place].alias);   
                opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset; 
                if (T->ptr[1]->kind == ARRAY_VIS) 
                    strcpy(opn1.id, strcat0("*", opn1.id));
                    
                result.kind = ID;
                strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
                result.offset = symbolTable.symbols[T->ptr[1]->place].offset;
                if (T->ptr[0]->kind == ARRAY_VIS) 
                    strcpy(result.id, strcat0("*", result.id));

                T->code = mergeCode(3, T->ptr[0]->code, T->ptr[1]->code, genIR(ASSIGNOP, opn1, opn2, result));
            }
            break;

        case INC:   //INC Exp     Exp INC  
        case DEC:       
            if (T->ptr[0]->kind != ID)
            {
                semantic_error(5, T->pos, "The left-hand side of an assignment must be a varible", "");
                break;
            }
            semantic_analysis(T->ptr[0]);
            T->ptr[0]->offset = T->offset;
            T->type = T->ptr[0]->type;
            T->width = T->ptr[0]->width; 

            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;

            opn2.kind = INT;
            opn2.const_int = 1;

            result.type = ID;
            strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
            result.offset = symbolTable.symbols[T->ptr[0]->place].offset;

            if (T->kind == INC)
                T->code = mergeCode(2, T->ptr[0]->code, genIR(PLUS, opn1, opn2, result));
            else if (T->kind == DEC)
                T->code = mergeCode(2, T->ptr[0]->code, genIR(MINUS, opn1, opn2, result));
            break;

        case AND:   //Exp AND Exp
            T->type = INT;
            T->ptr[0]->offset = T->offset;
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if (T->ptr[0]->type !=INT || T->ptr[1]->type != INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be int", "");
                break;
            }
            if(!check_type(T->ptr[0]->type, T->ptr[1]->type))
            {
                //printf("%d   %d\n", T->ptr[0]->type, T->ptr[1]->type);
                semantic_error(7, T->pos, "Type mismatched for operands", "");
                break;
            }

            /*生成中间代码
            T.code = T1.code || T2.code || ti=T1.alias * T2.alias
            || if ti==0 goto label1 || ti=1  || label1：
            */
            strcpy(label1, newLabel());
            T->code = mergeCode(2, T->ptr[0]->code, T->ptr[1]->code);

            //ti = T1.alias * T2.alias
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.type = symbolTable.symbols[T->ptr[0]->place].type;

            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn2.type = symbolTable.symbols[T->ptr[2]->place].type;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = symbolTable.symbols[T->place].type;           
            T->code = mergeCode(2, T->code, genIR(STAR, opn1, opn2, result));

            //if ti == 0 goto label1
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.type = symbolTable.symbols[T->ptr[0]->place].type;

            opn2.kind = INT;
            opn2.const_int = 0;
            opn2.type = INT;

            result.kind = ID;
            strcpy(result.id, label1);
            T->code = mergeCode(2, T->code, genIR(EQ, opn1, opn2, result));

            //ti = 1  || label1：
            opn1.kind = INT;
            opn1.const_int = 1;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = symbolTable.symbols[T->place].type;
            T->code = mergeCode(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));
            T->code = mergeCode(2, T->code, genLabel(label1));
            break;

        case OR:
            T->type = INT;
            T->ptr[0]->offset = T->offset;
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if (T->ptr[0]->type !=INT || T->ptr[1]->type != INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be int", "");
                break;
            }
            if(!check_type(T->ptr[0]->type, T->ptr[1]->type))
            {
                //printf("%d   %d\n", T->ptr[0]->type, T->ptr[1]->type);
                semantic_error(7, T->pos, "Type mismatched for operands", "");
                break;
            }

            /*生成中间代码
            T.code = T1.code || T2.code || ti = 0
            || if T1.alias == 0 goto label1 || ti = 1  || goto label2 || label1：
            || if T2.alias == 0 goto label2 || ti = 1  || label2：
            */
            strcpy(label1, newLabel());
            strcpy(label2, newLabel());
            T->code = mergeCode(2, T->ptr[0]->code, T->ptr[1]->code);

            //ti = 0
            opn1.kind = INT;
            opn1.const_int = 0;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = symbolTable.symbols[T->place].type;
            T->code = mergeCode(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));

            //if T1.alias == 0 goto label1
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.type = symbolTable.symbols[T->ptr[0]->place].type;

            opn2.kind = INT;
            opn2.const_int = 0;
            opn2.type = INT;

            result.kind = ID;
            strcpy(result.id, label1);
            T->code = mergeCode(2, T->code, genIR(EQ, opn1, opn2, result));

            //ti = 1 || goto label2 || label1：
            opn1.kind = INT;
            opn1.const_int = 1;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = symbolTable.symbols[T->place].type;
            T->code = mergeCode(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));
            T->code = mergeCode(3, T->code, genGoto(label2), genLabel(label1));

            //if T2.alias == 0 goto label2 
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[1]->place].alias);
            opn1.type = symbolTable.symbols[T->ptr[1]->place].type;

            opn2.kind = INT;
            opn2.const_int = 0;
            opn2.type = INT;

            result.kind = ID;
            strcpy(result.id, label2);
            T->code = mergeCode(2, T->code, genIR(EQ, opn1, opn2, result));

            //ti = 1  || label2：
            opn1.kind = INT;
            opn1.const_int = 1;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = symbolTable.symbols[T->place].type;
            T->code = mergeCode(3, T->code, genIR(ASSIGNOP, opn1, opn2, result), genLabel(label2));
            break;

        case RELOP:        
            T->type = INT;
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if (T->ptr[0]->type !=INT || T->ptr[1]->type != INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be int", "");
                break;
            }
            if(!check_type(T->ptr[0]->type, T->ptr[1]->type))
            {
                //printf("%d   %d\n", T->ptr[0]->type, T->ptr[1]->type);
                semantic_error(7, T->pos, "Type mismatched for operands", "");
                break;
            }

            /*  生成中间代码
            T.code = T1.code || T2.code
            || if T1.alias RELOP T2.alias goto label1
            || ti=0  || goto label2 || label1： ||  ti=1 || label2：
            */
            T->code = mergeCode(2, T->ptr[0]->code, T->ptr[1]->code);
            
            strcpy(label1, newLabel());
            strcpy(label2, newLabel());
            //if T1.alias RELOP T2.alias goto label1
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.type = symbolTable.symbols[T->ptr[0]->place].type;

            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
            opn2.type = symbolTable.symbols[T->ptr[1]->place].type;

            result.kind = ID;
            strcpy(result.id, label1);

            if (strcmp(T->type_id,"<") == 0)
                op = JLT;
            else if (strcmp(T->type_id,"<=") == 0)
                op = JLE;
            else if (strcmp(T->type_id,">") == 0)
                op = JGT;
            else if (strcmp(T->type_id,">=") == 0)
                op = JGE;
            else if (strcmp(T->type_id,"==") == 0)
                op = EQ;
            else if (strcmp(T->type_id,"!=") == 0)
                op = NEQ;
            
            T->code = mergeCode(2, T->code, genIR(op, opn1, opn2, result));

            //ti = 0 || goto label2
            opn1.kind = INT;
            opn1.const_int = 0;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            T->code = mergeCode(3, T->code, genIR(ASSIGNOP, opn1, opn2, result), genGoto(label2));

            //label1： ||  ti = 1 || label2：
            opn1.const_int = 1;
            T->code = mergeCode(3, T->code, genLabel(label1), genIR(ASSIGNOP, opn1, opn2, result));
            T->code = mergeCode(2, T->code, genLabel(label2));
            break;

        case ADD_ASSIGN:
        case SUB_ASSIGN:
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            Exp(T->ptr[0]);
            Exp(T->ptr[1]);
            if(!check_type(T->ptr[0]->type, T->ptr[1]->type))
            {
                //printf("%d   %d\n", T->ptr[0]->type, T->ptr[1]->type);
                semantic_error(5, T->pos, "Type mismatched for assignment", "");
                break;
            }
            T->type = T->ptr[0]->type;
            T->width = T->ptr[0]->width;
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.offset = symbolTable.symbols[T->ptr[0]->place].offset;

            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
            opn2.offset = symbolTable.symbols[T->ptr[1]->place].offset;

            result.type = ID;
            strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
            result.offset = symbolTable.symbols[T->ptr[0]->place].offset;

            if (T->kind == ADD_ASSIGN)
                T->code = mergeCode(3, T->ptr[0]->code, T->ptr[1]->code, genIR(PLUS, opn1, opn2, result));
            else if (T->kind == SUB_ASSIGN)
                T->code = mergeCode(3, T->ptr[0]->code, T->ptr[1]->code, genIR(MINUS, opn1, opn2, result));
            break;

            break;

        case PLUS:  //Exp PLUS Exp
        case MINUS:
        case STAR:
        case DIV:
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            T->ptr[1]->offset = T->offset + T->ptr[0]->width;
            Exp(T->ptr[1]);
            if(!check_type(T->ptr[0]->type, T->ptr[1]->type))
            {
                //printf("%d   %d\n", T->ptr[0]->type, T->ptr[1]->type);
                semantic_error(7, T->pos, "Type mismatched for operands", "");
                break;
            }
            //判断T->ptr[0]，T->ptr[1]类型是否正确，可能根据运算符生成不同形式的代码，给T的type赋值
            if (T->ptr[0]->type == FLOAT || T->ptr[1]->type == FLOAT)
            {
                T->type = FLOAT;
                T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
            }
            else if (T->ptr[0]->type == INT || T->ptr[1]->type == INT)
            {
                T->type = INT;
                T->width = T->ptr[0]->width + T->ptr[1]->width + 2;
            }
            else if (T->ptr[0]->type == CHAR || T->ptr[1]->type == CHAR)
            {
                T->type = CHAR;
                T->width = T->ptr[0]->width + T->ptr[1]->width + 1;
            }

            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
            //生成中间代码
            /*
            T.code = T1.code || T2.code || ti=T1.alias OP T2.alias
            */
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.type = T->ptr[0]->type;
            opn1.offset = symbolTable.symbols[T->ptr[1]->place].offset;
            if (T->ptr[0]->kind == ARRAY_VIS) 
                strcpy(opn1.id, strcat0("*", opn1.id));


            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[1]->place].alias);
            opn2.type = T->ptr[1]->type;
            opn2.offset = symbolTable.symbols[T->ptr[1]->place].offset;
            if (T->ptr[1]->kind == ARRAY_VIS) 
                strcpy(opn2.id, strcat0("*", opn2.id));

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = T->type;
            result.offset = symbolTable.symbols[T->place].offset;

            T->code = mergeCode(3, T->ptr[0]->code, T->ptr[1]->code, 
                genIR(T->kind, opn1, opn2, result));

            if (T->type == INT)       
                T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
            else if (T->type == FLOAT)
                T->width = T->ptr[0]->width + T->ptr[1]->width + 8;
            else if (T->type == CHAR)
                T->width = T->ptr[0]->width + T->ptr[1]->width + 1;
            break;

        case NOT:
            T->type = INT;
            T->ptr[0]->offset = T->offset;
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            Exp(T->ptr[0]);
            if (T->ptr[0]->type !=INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
                break;
            }
            /*
            T.code = if T1.alias == 0 goto label1
            || ti = 0 || goto label2 || label1：||  ti = 1 || label2：
            */
            strcpy(label1, newLabel());
            strcpy(label2, newLabel());
            //if T1.alias == 0 goto label1
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn1.type = symbolTable.symbols[T->ptr[0]->place].type;

            opn2.kind = INT;
            opn2.const_int = 0;
            opn2.type = INT;

            result.kind = ID;
            strcpy(result.id, label1);
            T->code = mergeCode(2, T->code, genIR(EQ, opn1, opn2, result));

            //ti = 0 || goto label2 || label1：
            opn1.kind = INT;
            opn1.const_int = 0;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = INT;
            T->code = mergeCode(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));
            T->code = mergeCode(3, T->code, genGoto(label2), genLabel(label1));

            //ti = 1 || label2：
            opn1.kind = INT;
            opn1.const_int = 1;
            opn1.type = INT;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = INT;
            T->code = mergeCode(2, T->code, genIR(ASSIGNOP, opn1, opn2, result));
            T->code = mergeCode(2, T->code, genLabel(label2));
            break;

        case UMINUS:
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            if (T->ptr[0]->type == FLOAT)
            {
                T->type = FLOAT;
                T->width = T->ptr[0]->width + 4;
            }
            else if (T->ptr[0]->type == INT)
            {
                T->type = INT;
                T->width = T->ptr[0]->width + 2;
            }
            else if (T->ptr[0]->type == CHAR)
            {
                T->type = CHAR;
                T->width = T->ptr[0]->width + 1;
            }
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->ptr[0]->width);
            
            //T.code = T1.code || ti = 0 - T1.alias
            opn1.kind = INT;
            opn1.const_int = 0;

            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[T->ptr[0]->place].alias);
            opn2.type = symbolTable.symbols[T->ptr[0]->place].type;
            opn2.offset = symbolTable.symbols[T->ptr[0]->place].offset;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.type = symbolTable.symbols[T->place].type;
            result.offset = symbolTable.symbols[T->place].offset;
            T->code = mergeCode(2, T->ptr[0]->code, genIR(MINUS, opn1, opn2, result));
            break;

        case FUNC_CALL: //ID LP Args RP     ID LP RP
            //根据T->type_id查出函数的定义，如果语言中增加了实验教材的read，write需要单独处理一下
            rtn = searchSymbolTable(T->type_id);
            if (rtn == -1)
            {
                semantic_error(2, T->pos, "Undefined function", T->type_id);
                break;
            }
            if (symbolTable.symbols[rtn].flag != 'F')
            {
                semantic_error(13, T->pos, T->type_id, "is not a function");
                break;
            }

            T->type = symbolTable.symbols[rtn].type;
            //width存放函数返回值的单数字节数
            width = get_width(T->type);

            //处理函数参数
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]);     //处理所有实参表达式求值，及类型
                T->width = T->ptr[0]->width + width;    //累加上计算实参使用临时变量的单元数
                //T.code = T1.code;
                T->code = T->ptr[0]->code;              
            }
            else
            {
                T->width = width;
                T->code = NULL;     //没有参数
            }
            match_param(rtn, T);

            T0 = T->ptr[0];
            //处理write
            if (strcmp(T->type_id, "write") == 0)
            {
                opn1.kind = ID;
                strcpy(opn1.id, T->type_id);
                opn1.offset = rtn;

                result.kind = ID;
                strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);
                T->code = mergeCode(2, T->code, genIR(CALL, opn1, opn2, result));
                break;
            }

            while (T0 != NULL)
            {
                result.kind = ID;
                strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);
                //T.code = T. code || ARG T01.alias
                T->code = mergeCode(2, T->code, genIR(ARG, opn1, opn2, result));
                T0 = T0->ptr[1];
            }
            
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->width - width);
            
            

            //中间代码生成 T.code = T.code || ti = CALL 函数名
            opn1.kind = ID;
            strcpy(opn1.id, T->type_id);
            opn1.offset = rtn;

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            result.offset = symbolTable.symbols[T->place].offset;
            T->code = mergeCode(2, T->code, genIR(CALL, opn1, opn2, result));
            break;

        case ARGS: //Exp COMMA Args    Exp
            //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            /*中间代码生成
            if（T2==NULL）
                T.code = T1.code 
            else 
                T.code= T1.code || T2.code 
            */
            T->code = T->ptr[0]->code;
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                Exp(T->ptr[1]);
                T->width += T->ptr[1]->width;
                T->code = mergeCode(2, T->code, T->ptr[1]->code);
            } 
            break;       

        case ARRAY_VIS:     //ID LT Exp RT
            T->ptr[0]->offset = T->offset;
            rtn = rtn2 = searchSymbolTable(T->type_id);
            if (rtn == -1)
            {
                semantic_error(1, T->pos, "Undefined varible", T->type_id);
                break;
            }                          
            if (symbolTable.symbols[rtn].flag == 'F')
            {
                semantic_error(11, T->pos, T->type_id, "is a function, not a varible");
                break;
            }
            T->array_size = symbolTable.symbols[rtn].paramnum;
            T->type = symbolTable.symbols[rtn].type;
            if (T->array_size == 0)
            {
                semantic_error(17, T->pos, T->type_id, "is not an array");
                break;
            }
            Exp(T->ptr[0]);
            if (T->ptr[0]->type != INT)
            {
                semantic_error(15, T->pos, "Array index must be an integer", "");
                break;
            }
            if (T->ptr[0]->kind == INT && T->ptr[0]->type_int >= T->array_size)
            {
                semantic_error(18, T->pos, "Index out of bound for array", T->type_id);
                break;
            }
            T->width = T->ptr[0]->width;
            

            // temp1 = t1.alias * 4 || temp2 = &v + temp1  
            rtn = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);

            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[T->ptr[0]->place].alias);

            opn2.kind = INT;
            opn2.const_int = get_width(T->type);

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[rtn].alias);
            T->code = mergeCode(2, T->ptr[0]->code, genIR(STAR, opn1, opn2, result));
            
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            opn1.kind = ID;
            strcpy(opn1.id, symbolTable.symbols[rtn2].alias);
            strcpy(opn1.id, strcat0("&", opn1.id));

            opn2.kind = ID;
            strcpy(opn2.id, symbolTable.symbols[rtn].alias);

            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[T->place].alias);
            T->code = mergeCode(2, T->code, genIR(PLUS, opn1, opn2, result));
            
            break;

        default:
            break;        
        }
    }
}


void semantic_analysis(struct ASTNode *T)
{
	//对抽象语法树的先根遍历,按display的控制结构修改完成符号表管理和语义检查和TAC生成（语句部分）
    int rtn, num, width;
    struct ASTNode *T0;
    OPN opn1, opn2, result;

    if (T)
    {
        switch (T->kind)
        {
        case EXT_DEF_LIST:  //ExtDef ExtDefList
            if (!T->ptr[0])
                break;
            T->ptr[0]->offset = T->offset;
            semantic_analysis(T->ptr[0]);       //访问外部定义列表中的第一个
            T->code = T->ptr[0]->code;
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
                semantic_analysis(T->ptr[1]);   //访问该外部定义列表中的其它外部定义
                T->code = mergeCode(2, T->code, T->ptr[1]->code);
            }
            break;

        case EXT_VAR_DEF:  //Specifier ExtDecList SEMI 
            T->type = get_type(T->ptr[0]->type_id);
            T->ptr[1]->type = T->type;
            T->ptr[1]->offset = T->offset;                 
            T->ptr[1]->width = get_width(T->type);  

            ext_var_list(T->ptr[1]);
            T->width = get_width(T->type) * T->ptr[1]->num;
            //这里假定外部变量不支持初始化 
            T->code = T->ptr[1]->code; 
            break;

        case FUNC_DEF:  //Specifier FuncDec CompSt
            T->ptr[1]->type = get_type(T->ptr[0]->type_id);
            T->width = 0;     //函数的宽度设置为0，不会对外部变量的地址分配产生影响
            T->offset = DX;   //设置局部变量在活动记录中的偏移量初值
            semantic_analysis(T->ptr[1]); 

            T->offset += T->ptr[1]->width;   //用形参单元宽度修改函数局部变量的起始偏移量            
            T->ptr[2]->offset = T->offset;

            //T3.Snext = newLabel
            strcpy(T->ptr[2]->Snext, newLabel());

            semantic_analysis(T->ptr[2]);         //处理函数体结点CompSt
            //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
            symbolTable.symbols[T->ptr[1]->place].offset = T->offset + T->ptr[2]->width;

            //T.code = T2.code || T3.code || T3.Snext
            T->code = mergeCode(3, T->ptr[1]->code, T->ptr[2]->code, genLabel(T->ptr[2]->Snext));
            break;

        case FUNC_DEC:  //ID LP VarList RP     ID LP RP
            //根据返回类型，函数名填写符号表
            //函数不在数据区中分配单元，偏移量为0
            rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'F', 0);
            if (rtn == -1)
            {
                semantic_error(4, T->pos, "Redefined function", T->type_id);
                break;
            }
            else
                T->place = rtn;

            //T.code = FUNCTION 函数名 
            result.kind = ID;
            strcpy(result.id, T->type_id);
            result.offset = rtn;           
            T->code = genIR(FUNCTION, opn1, opn2, result);
            T->offset = DX; //设置形式参数在活动记录中的偏移量初值 

            //判断是否有参数 if  (T1非空) T.code = T.code || T1.code
            if (T->ptr[0])
            {                
                T->ptr[0]->offset = T->offset;
                //处理函数参数列表
                semantic_analysis(T->ptr[0]);
                T->width = T->ptr[0]->width;
                symbolTable.symbols[rtn].paramnum = T->ptr[0]->num;
                T->code = mergeCode(2, T->code, T->ptr[0]->code);               
            }
            else
            {
                symbolTable.symbols[rtn].paramnum = 0;
                T->width = 0;
            }
            break;

        case PARAM_LIST:    //ParamDec        ParamDec COMMA VarList
            //处理函数形式参数列表
            T->ptr[0]->offset = T->offset;
            semantic_analysis(T->ptr[0]);
            //判断是否还有其他参数 T.code = T1.code || T2.code
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                semantic_analysis(T->ptr[1]);
                T->num = T->ptr[0]->num + T->ptr[1]->num;
                T->width = T->ptr[0]->width + T->ptr[1]->width;
                T->code = mergeCode(2, T->ptr[0]->code, T->ptr[1]->code);
            }
            else  //T.code = T1.code
            {
                T->num = T->ptr[0]->num;
                T->width = T->ptr[0]->width;
                T->code = T->ptr[0]->code;
            }
            break;

        case PARAM_DEC: //Specifier VarDec
            rtn = fillSymbolTable(T->ptr[1]->type_id, newAlias(), 1, T->ptr[0]->type, 'P', T->offset);
            if (rtn == -1)
                semantic_error(3, T->ptr[1]->pos, "Redefined argument", T->ptr[1]->type_id);
            else
                T->ptr[1]->place = rtn;
            T->num = 1;
            T->width = get_width(T->ptr[0]->type);

            //T.code = PARAM T2.alias 
            result.kind = ID;
            strcpy(result.id, symbolTable.symbols[rtn].alias);
            T->code = genIR(PARAM, opn1, opn2, result);
            break;

        case COMP_STM:  //LC DefList StmList RC
            //设置层号加1，
            LEV++;
            //保存该层局部变量在符号表中的起始位置在symbol_scope_TX
            symbol_scope_TX.TX[symbol_scope_TX.top++] = symbolTable.index;
            T->width = 0;
            T->code = NULL;
            // T.code = T1.code || T2.code
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset + T->width;
                //处理复合语句的语句序列
                semantic_analysis(T->ptr[0]);
                T->width += T->ptr[0]->width; 
                T->code = T->ptr[0]->code;
            }
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->width;
                //T2.Snext = T.Snext
                strcpy(T->ptr[1]->Snext, T->Snext);
                semantic_analysis(T->ptr[1]);
                T->width += T->ptr[1]->width;
                T->code = mergeCode(2, T->code, T->ptr[1]->code);
            }
            //c在退出一个符合语句前显示的符号表
            //if (err_num == 0)              
            //print_symbols();
            //出复合语句，层号减1
            LEV--;           
            //删除该作用域中的符号
            symbolTable.index = symbol_scope_TX.TX[--symbol_scope_TX.top];
            break;

        case DEF_LIST:  //Def DefList  | NULL  
            T->code = NULL;
            // T.code = T1.code || T2.code
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset;
                semantic_analysis(T->ptr[0]);
                T->width = T->ptr[0]->width;
                T->code = T->ptr[0]->code;
            }

            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                //处理剩下的局部变量定义
                semantic_analysis(T->ptr[1]);   
                T->width += T->ptr[1]->width; 
                T->code = mergeCode(2, T->code, T->ptr[1]->code);           
            }
            break;

        case VAR_DEF:   //Specifier DecList SEMI
            //处理一个局部变量定义,将第一个孩子(TYPE结点)中的类型送到第二个孩子的类型域
            //类似于上面的外部变量EXT_VAR_DEF，换了一种处理方法
            T->code = NULL;
            T->ptr[1]->type = get_type(T->ptr[0]->type_id);
            //T0为变量名列表子树根指针，对ID、ASSIGNOP类结点在登记到符号表，作为局部变量
            T0 = T->ptr[1];
            num = 0;
            T0->offset = T->offset;
            T->width = 0;
            width = get_width(T->ptr[1]->type);

            while (T0)
            {
                //处理所有DEC_LIST结点
                /*
                DecList : Dec                           
                        | Dec COMMA DecList;

                    Dec : VarDec                        
                        | VarDec ASSIGNOP Exp;
                */
                num++;
                //类型属性向下传递
                T0->ptr[0]->type = T0->type;  
                if (T0->ptr[1]) 
                    T0->ptr[1]->type = T0->type;
                //类型属性向下传递
                T0->ptr[0]->offset = T0->offset;  
                if (T0->ptr[1]) 
                    T0->ptr[1]->offset = T0->offset + width;

                if (T0->ptr[0]->kind == ID)
                {
                    //此处偏移量未计算，暂时为0
                    rtn = fillSymbolTable(T0->ptr[0]->type_id,newAlias(), LEV, 
                        T0->ptr[0]->type, 'V', T->offset + T->width);
                    if (rtn == -1)
                        semantic_error(3, T0->ptr[0]->pos, "Redefined varible", T0->ptr[0]->type_id);
                    else
                        T0->ptr[0]->place = rtn;
                    T->width += width;
                } 
                else if (T0->ptr[0]->kind == ARRAY_DEC)
                {
                    rtn = fillSymbolTable(T0->ptr[0]->type_id, newAlias(), LEV, T0->ptr[0]->type,
                         'V', T->offset + T->width);
                    if (rtn == -1)
                    {
                        semantic_error(3, T0->ptr[0]->pos, "Redefined varible", T0->ptr[0]->type_id);
                        break;
                    }                    
                    if (T0->ptr[0]->array_size == 0)
                    {
                        semantic_error(14, T->pos, "The length of an array must above zero", "");
                        break;
                    }
                    T0->ptr[0]->place = rtn;
                    symbolTable.symbols[rtn].paramnum = T0->ptr[0]->array_size;
                    T->width += width;
                    opn1.kind = INT;
                    opn1.const_int = symbolTable.symbols[T0->ptr[0]->place].paramnum * get_width(T0->ptr[0]->type);
                    
                    result.kind = ID;
                    strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);
                    T->code = mergeCode(2, T->code, genIR(ARRAY_DEC, opn1, opn2, result));
                }  
                else if (T0->ptr[0]->kind == ASSIGNOP)
                {   
                    //此处偏移量未计算，暂时为0
                    rtn = fillSymbolTable(T0->ptr[0]->ptr[0]->type_id, newAlias(), LEV, 
                        T0->ptr[0]->type, 'V', T->offset + T->width);
                    if (rtn == -1)
                        semantic_error(3, T0->ptr[0]->ptr[0]->pos, 
                            "Redefined varible", T0->ptr[0]->ptr[0]->type_id);
                    else
                    {
                        T0->ptr[0]->place = rtn;
                        T0->ptr[0]->ptr[1]->offset = T->offset + T->width + width;
                        Exp(T0->ptr[0]->ptr[1]);
                        
                        opn1.kind = ID;
                        strcpy(opn1.id, symbolTable.symbols[T0->ptr[0]->ptr[1]->place].alias);
                        
                        result.kind = ID;
                        strcpy(result.id, symbolTable.symbols[T0->ptr[0]->place].alias);

                        T->code = mergeCode(3, T->code, T0->ptr[0]->ptr[1]->code, genIR(ASSIGNOP, opn1, opn2, result));
                    }
                    T->width += width + T0->ptr[0]->ptr[1]->width;
                }
                T0 = T0->ptr[1];
            }
            break;

        case STM_LIST:  //Stmt StmList 
            if (!T->ptr[0])
            {
                T->code = NULL;
                T->width = 0;
                break;
            }
            //空语句序列
            /*
            if(T2非空)
            {
                T1.Snext = newLabel;
                T2.Snext = T.Snext;
            }
            else 
                T1.Snext = S.next;
            */
            if (T->ptr[1])
            {
                strcpy(T->ptr[0]->Snext, newLabel());
                strcpy(T->ptr[1]->Snext, T->Snext);
            }  
            else
                strcpy(T->ptr[0]->Snext, T->Snext);

            T->ptr[0]->offset = T->offset;
            semantic_analysis(T->ptr[0]);
            T->code = T->ptr[0]->code;
            T->width = T->ptr[0]->width;
            if (T->ptr[1])
            {
                
                //顺序结构共享单元方式
                T->ptr[1]->offset = T->offset;
                semantic_analysis(T->ptr[1]);
                /*
                访问T的所有子树后：
                if (T2为空)  
                    T.code = T1.code 
                else         
                    T.code = T1.Snext || T1.Snext || T2.code
                */
                if (T->ptr[0]->kind == RETURN || T->ptr[0]->kind == EXP_STMT || T->ptr[0]->kind == COMP_STM)
                    T->code = mergeCode(2, T->code, T->ptr[1]->code);
                else
                    T->code = mergeCode(3, T->code, genLabel(T->ptr[0]->Snext), T->ptr[1]->code);

                //顺序结构共享单元方式
                if (T->ptr[1]->width > T->width)
                    T->width = T->ptr[1]->width;
            }
            break;

        case IF_THEN:  //IF LP Exp RP Stmt
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset; 
            /*
            T1.Etrue = newLabel, T1.Efalse = T2.Snext = T.Snext
            */
            strcpy(T->ptr[0]->Etrue, newLabel());
            strcpy(T->ptr[0]->Efalse, T->Snext);
            strcpy(T->ptr[1]->Snext, T->Snext);

            //semantic_analysis(T->ptr[0]);
            translate_boolExp(T->ptr[0]);      
            T->width = T->ptr[0]->width;
            semantic_analysis(T->ptr[1]);

            //T.code = T1.code || T1.Etrue || T2.code
            T->code = mergeCode(3, T->ptr[0]->code, genLabel(T->ptr[0]->Etrue), T->ptr[1]->code);

            if (T->width < T->ptr[1]->width) 
                T->width = T->ptr[1]->width;
            break;

        case IF_THEN_ELSE:  //IF LP Exp RP Stmt ELSE Stmt
            T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->offset;  
            /*
            T1.Etrue = newLabel, T1.Efalse = T.Snext
            T2.Snext = T3.Snext = T.Snext
            */
            strcpy(T->ptr[0]->Etrue, newLabel());
            strcpy(T->ptr[0]->Efalse, T->Snext);
            strcpy(T->ptr[1]->Snext, T->Snext);
            strcpy(T->ptr[2]->Snext, T->Snext);

            translate_boolExp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            //semantic_analysis(T->ptr[0]);
            //if子句
            semantic_analysis(T->ptr[1]);                 
            if (T->width < T->ptr[1]->width) 
                T->width = T->ptr[1]->width;   
            //else子句            
            semantic_analysis(T->ptr[2]);                
            if (T->width < T->ptr[2]->width) 
                T->width = T->ptr[2]->width;

            /*
            T.code = T1.code || T1.Etrue || T2.code || goto T.Enext
            || T1.Efalse || T3.code 
            */
            T->code = mergeCode(6, T->ptr[0]->code, 
                                genLabel(T->ptr[0]->Etrue),
                                T->ptr[1]->code, 
                                genGoto(T->Snext), 
                                genLabel(T->ptr[0]->Efalse),
                                T->ptr[2]->code);
            break;

        case WHILE:  //WHILE LP Exp RP Stmt
            start++;
            /*
            T1.Etrue = newLabel, T1.Efalse = T.Snext,
            T2.Snext = newLabel;
            */
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
            strcpy(T->ptr[0]->Etrue, newLabel());
            strcpy(T->ptr[0]->Efalse, T->Snext);
            strcpy(T->ptr[1]->Snext, newLabel());         
            
            //Exp(T->ptr[0]);
            translate_boolExp(T->ptr[0]);

            T->width = T->ptr[0]->width;
            semantic_analysis(T->ptr[1]);      //循环体
            if (T->width < T->ptr[1]->width) 
                T->width = T->ptr[1]->width;

            //T.code = T2.Snext || T1.code || T1.Etrue || T2.code || goto T2.Snext
            T->code = mergeCode(5, genLabel(T->ptr[1]->Snext),
                                T->ptr[0]->code,
                                genLabel(T->ptr[0]->Etrue),
                                T->ptr[1]->code,
                                genGoto(T->ptr[1]->Snext));

            end++;     
            break;
        
        case BREAK:
        case CONTINUE:
            if (start <= end)
            {
                semantic_error(19, T->pos, "\'break\' or \'continue\' must be in a loop", "");
                break;
            }
            break;
        
        case EXP_STMT:  //Exp SEMI
            T->ptr[0]->offset = T->offset;
            //T1.Snext = T.Snext;
            strcpy(T->ptr[0]->Snext, T->Snext);
            semantic_analysis(T->ptr[0]);
            T->width = T->ptr[0]->width;
            //T.code = T1.code
            T->code = T->ptr[0]->code;
            break;

        case RETURN:  //RETURN Exp SEMI
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset;
                //T1.Snext = T.Snext;
                strcpy(T->ptr[0]->Snext, T->Snext);
                Exp(T->ptr[0]);  

                //判断返回值类型是否匹配
                num = symbolTable.index - 1;
                while (symbolTable.symbols[num].flag != 'F')
                    num--;
                if (T->ptr[0]->type != symbolTable.symbols[num].type)
                {
                    semantic_error(8, T->pos, "Type mismatched for return", "");
                    T->width = 0;
                    break;
                }

               T->width = T->ptr[0]->width;

               //T.code = T1.code || return T1.alias
               result.kind = ID;
               strcpy(result.id, symbolTable.symbols[T->ptr[0]->place].alias);
               result.offset = symbolTable.symbols[T->ptr[0]->place].offset;
               T->code = mergeCode(2, T->ptr[0]->code, genIR(RETURN, opn1, opn2, result));
            }
            else
            {
                T->width = 0;
                //T.code = T1.code || return
                result.kind = 0;
                T->code = mergeCode(RETURN, opn1, opn2, result);
            }
            break;

        case ID:
        case INT:
        case FLOAT:
        case ASSIGNOP:
        case AND:
        case OR:
        case RELOP:
        case PLUS:
        case MINUS:
        case STAR:
        case DIV:
        case ADD_ASSIGN:
        case SUB_ASSIGN:
        case INC:
        case DEC:
        case NOT:
        case UMINUS:
        case FUNC_CALL:
        case ARRAY_VIS:
            Exp(T);          //处理基本表达式
            break;

        default:
            break;
        }
    }
}


void semantic_analysis0(struct ASTNode *T)
{
	symbolTable.index = 0;
	//将read和write函数添加到符号表中
    fillSymbolTable("read", "", 0, INT, 'F', 4);
    symbolTable.symbols[0].paramnum = 0; //read的形参个数为0
    fillSymbolTable("write", "", 0, INT, 'F', 4);
    symbolTable.symbols[1].paramnum = 1; //write形参个数为1
    fillSymbolTable("x", "", 1, INT, 'P', 12);
	
    symbol_scope_TX.TX[0] = 0;  //外部变量在符号表中的起始序号为0
    symbol_scope_TX.top = 1;
    T->offset = 0;              //外部变量在数据区的偏移量	
    semantic_analysis(T);
    //if (err_num == 0)
      //  print_symbols();
    printIR(T->code);
}




