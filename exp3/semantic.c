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
    return strcat0("temp", s);
}

int get_width(int type)
{
    if (type == INT)
        return 4;
    else if (type == FLOAT)
        return 8;
    else if (type == CHAR)
        return 1;
    else
        return -1;
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
	if(T == NULL)
		return;
	
	int rtn, num;
	switch(T->kind)
	{	
	case EXT_DEC_LIST:     //VarDec COMMA ExtDecList
		T->ptr[0]->type = T->type;					//将类型属性向下传递变量结点
		T->ptr[0]->offset = T->offset;    			//外部变量的偏移量向下传递
		T->ptr[1]->type = T->type;             		//将类型属性向下传递变量结点
        T->ptr[1]->offset = T->offset + T->width; 	//外部变量的偏移量向下传递
        T->ptr[1]->width = T->width;
		ext_var_list(T->ptr[0]);
        ext_var_list(T->ptr[1]);
        T->num = T->ptr[1]->num + 1;				//统计参数个数
        break;
		
	case ID:
		rtn = fillSymbolTable(T->type_id, newAlias(), LEV, T->type, 'V', T->offset);  //最后一个变量名
		if (rtn == -1)
			semantic_error(3, T->pos, "Redefined varible", T->type_id);
		else 
			T->place = rtn;
        //printf("%d\n", T->array_size);
		T->num = 1;
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
        T->num = 1;
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

//处理基本表达式
void Exp(struct ASTNode *T)
{
	int rtn, num, width;
    struct ASTNode *T0;

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
                //T->code = NULL;     //标识符不需要生成TAC
                T->type = symbolTable.symbols[rtn].type;
                T->offset = symbolTable.symbols[rtn].offset;
                T->width = 0;       //未再使用新单元
            }
            //TODO中间代码生成
            break;

        case INT:
            //为整型常量生成一个临时变量
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            //T->place = fill_Temp("int const", LEV, T->type, 'T', T->offset);
            T->type = INT;
            T->width = 4;
            //TODO中间代码生成
            break;

        case FLOAT:
            //为浮点常量生成一个临时变量
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            T->type = FLOAT;
            T->width = 4;
            //TODO中间代码生成
            break;

        case CHAR:
            //为字符常量生成一个临时变量
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset);
            T->type = CHAR;
            T->width = 1;
            //TODO中间代码生成
            break;

        case ASSIGNOP:  //Exp ASSIGNOP Exp
            if (!(T->ptr[0]->kind == ID || T->ptr[0]->kind == ARRAY_DEC || T->ptr[0]->kind == ARRAY_VIS))           
                semantic_error(6, T->pos, "The left-hand side of an assignment must be a varible", "");
            else
            {
                Exp(T->ptr[0]);
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
                //TODO中间代码生成
                T->code = NULL;                 
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
            break;

        case AND:   //Exp AND Exp
        case OR:
        case RELOP:        
            T->type = INT;
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;
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
            //T->place = fill_Temp("operands", LEV, T->type, 'T', T->offset + T->ptr[0]->width + T->ptr[1]->width);
            if (T->type == INT)       
                T->width = T->ptr[0]->width + T->ptr[1]->width + 4;
            else if (T->type == FLOAT)
                T->width = T->ptr[0]->width + T->ptr[1]->width + 8;
            else if (T->type == CHAR)
                T->width = T->ptr[0]->width + T->ptr[1]->width + 1;
            //TODO中间代码生成
            break;

        case NOT:
            T->type = INT;
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            if (T->ptr[0]->type !=INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
                break;
            }
            if (T->ptr[0]->type != INT)
            {
                semantic_error(7, T->pos, "Type mismatched for operands", "");
                break;
            }           
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
                //TODO中间代码生成               
            }
            else
            {
                T->width = width;
                //TODO中间代码生成
            }
            match_param(rtn, T); 
            T->place = fill_Temp(newTemp(), LEV, T->type, 'T', T->offset + T->width - width);
            //TODO中间代码生成
            break;

        case ARGS: //Exp COMMA Args    Exp
            //此处仅处理各实参表达式的求值的代码序列，不生成ARG的实参系列
            T->ptr[0]->offset = T->offset;
            Exp(T->ptr[0]);
            T->width = T->ptr[0]->width;
            //TODO中间代码生成
            break;       

        case ARRAY_VIS:     //ID LT Exp RT
            T->ptr[0]->offset = T->offset;
            rtn = searchSymbolTable(T->type_id);
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

    if (T)
    {
        switch (T->kind)
        {
        case EXT_DEF_LIST:
            if (!T->ptr[0])
                break;
            T->ptr[0]->offset = T->offset;
            semantic_analysis(T->ptr[0]);       //访问外部定义列表中的第一个
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->ptr[0]->offset + T->ptr[0]->width;
                semantic_analysis(T->ptr[1]);   //访问该外部定义列表中的其它外部定义
            }
            break;

        case EXT_VAR_DEF:  //Specifier ExtDecList SEMI 
            //处理外部说明,将标识符中的类型送到变量定义的类型域
            T->type = get_type(T->ptr[0]->type_id);
            T->ptr[1]->type = T->type;
            //这个外部变量的偏移量向下传递
            T->ptr[1]->offset = T->offset;        
            //将一个变量的宽度向下传递            
            T->ptr[1]->width = get_width(T->type);                     
            //处理外部变量说明中的标识符序列
            ext_var_list(T->ptr[1]);
            //计算这个外部变量说明的宽度
            T->width = get_width(T->type) * T->ptr[1]->num; 
            break;

        case FUNC_DEF:  //Specifier FuncDec CompSt
            //填写函数定义信息到符号表
            T->ptr[1]->type = get_type(T->ptr[0]->type_id);
            T->width = 0;     //函数的宽度设置为0，不会对外部变量的地址分配产生影响
            T->offset = DX;   //设置局部变量在活动记录中的偏移量初值
            semantic_analysis(T->ptr[1]); //处理函数名和参数结点(FUNC_DEC)部分，这里不考虑用寄存器传递参数

            T->offset += T->ptr[1]->width;   //用形参单元宽度修改函数局部变量的起始偏移量
            T->ptr[2]->offset = T->offset;
            semantic_analysis(T->ptr[2]);         //处理函数体结点CompSt
            //TODO中间代码生成
            //计算活动记录大小,这里offset属性存放的是活动记录大小，不是偏移
            symbolTable.symbols[T->ptr[1]->place].offset = T->offset + T->ptr[2]->width;
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

            T->offset = DX;
            //设置形式参数在活动记录中的偏移量初值
            //判断是否有参数
            if (T->ptr[0])
            {
                
                T->ptr[0]->offset = T->offset;
                //处理函数参数列表
                semantic_analysis(T->ptr[0]);
                T->width = T->ptr[0]->width;
                symbolTable.symbols[rtn].paramnum = T->ptr[0]->num;               
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
            //判断是否还有其他参数
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                semantic_analysis(T->ptr[1]);
                T->num = T->ptr[0]->num + T->ptr[1]->num;
                T->width = T->ptr[0]->width + T->ptr[1]->width;
                //TODO中间代码生成
            }
            else
            {
                T->num = T->ptr[0]->num;
                T->width = T->ptr[0]->width;
                //TODO中间代码生成
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
            //TODO中间代码生成
            break;

        case COMP_STM:  //LC DefList StmList RC
            //设置层号加1，
            LEV++;
            //保存该层局部变量在符号表中的起始位置在symbol_scope_TX
            symbol_scope_TX.TX[symbol_scope_TX.top++] = symbolTable.index;
            T->width = 0;
            T->code = NULL;
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset + T->width;
                //处理复合语句的语句序列
                semantic_analysis(T->ptr[0]);
                T->width += T->ptr[0]->width; 
                //TODO中间代码生成
            }
            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->width;
                semantic_analysis(T->ptr[1]);
                T->width += T->ptr[1]->width;
            }
            //c在退出一个符合语句前显示的符号表
            if (err_num == 0)              
                print_symbols();
            //出复合语句，层号减1
            LEV--;           
            //删除该作用域中的符号
            symbolTable.index = symbol_scope_TX.TX[--symbol_scope_TX.top];
            break;

        case DEF_LIST:  //Def DefList
            T->code = NULL;
            if (T->ptr[0])
            {
                T->ptr[0]->offset = T->offset;
                semantic_analysis(T->ptr[0]);
                T->width = T->ptr[0]->width;
                //TODO中间代码生成
            }

            if (T->ptr[1])
            {
                T->ptr[1]->offset = T->offset + T->ptr[0]->width;
                //处理剩下的局部变量定义
                semantic_analysis(T->ptr[1]);   
                T->width += T->ptr[1]->width; 
                //TODO中间代码生成           
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
                int i, j = 12 / 3;
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
                        //TODO中间代码生成
                    }
                    T->width += width + T0->ptr[0]->ptr[1]->width;
                }
                T0 = T0->ptr[1];
            }
            break;

        case STM_LIST:  //Stmt StmList 
            if (!T->ptr[0])
            {
                T->width = 0;
                break;
            }
            //空语句序列
            //TODO中间代码生成

            T->ptr[0]->offset = T->offset;
            semantic_analysis(T->ptr[0]);
            T->width = T->ptr[0]->width;

            if (T->ptr[1])
            {
                //TODO中间代码生成
                //顺序结构共享单元方式
                T->ptr[1]->offset = T->offset;
                semantic_analysis(T->ptr[1]);
                //TODO中间代码生成
                //顺序结构共享单元方式
                if (T->ptr[1]->width > T->width)
                    T->width = T->ptr[1]->width;
            }
            break;

        case IF_THEN:  //IF LP Exp RP Stmt
            //TODO中间代码生成
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset; 
            semantic_analysis(T->ptr[0]);
            if (T->ptr[0]->type !=INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
                break;
            }         
            //T->width = T->ptr[0]->width;
            T->width = 0;
            semantic_analysis(T->ptr[1]);
            if (T->width < T->ptr[1]->width) 
                T->width = T->ptr[1]->width;
            break;

        case IF_THEN_ELSE:  //IF LP Exp RP Stmt ELSE Stmt
            T->ptr[0]->offset = T->ptr[1]->offset = T->ptr[2]->offset = T->offset;
            //TODO中间代码生成
            //T->width = T->ptr[0]->width;
            semantic_analysis(T->ptr[0]);
            if (T->ptr[0]->type != INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
                break;
            }
            T->width = 0;   //处理完中间代码删掉
            //if子句
            semantic_analysis(T->ptr[1]);                 
            if (T->width < T->ptr[1]->width) 
                T->width = T->ptr[1]->width;   
            //else子句            
            semantic_analysis(T->ptr[2]);                
            if (T->width < T->ptr[2]->width) 
                T->width = T->ptr[2]->width;
            break;

        case WHILE:  //WHILE LP Exp RP Stmt
            //TODO中间代码生成
            start++;
            T->ptr[0]->offset = T->ptr[1]->offset = T->offset;         
            //T->width = T->ptr[0]->width;
            Exp(T->ptr[0]);
            if (T->ptr[0]->type != INT)
            {
                semantic_error(16, T->pos, "The type of a bool expression must be integer", "");
                break;
            }
            T->width = 0;   //处理完中间代码删掉                   
            semantic_analysis(T->ptr[1]);      //循环体
            if (T->width < T->ptr[1]->width) 
                T->width = T->ptr[1]->width;    
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
            semantic_analysis(T->ptr[0]);
            //TODO中间代码生成
            T->width = T->ptr[0]->width;
            break;

        case RETURN:  //RETURN Exp SEMI
            if (T->ptr[0])
            {
                //TODO中间代码生成
                T->ptr[0]->offset = T->offset;
                Exp(T->ptr[0]);              
                //判断返回值类型是否匹配
                num = symbolTable.index - 1;
                while (symbolTable.symbols[num].flag != 'F')
                    num--;
                if (T->ptr[0]->type != symbolTable.symbols[num].type)
                {
                    //printf("%d\n", T->ptr[0]->type);
                    //printf("%d\n", symbolTable.symbols[num].type);
                    semantic_error(8, T->pos, "Type mismatched for return", "");
                    T->width = 0;
                    break;
                }

               T->width = T->ptr[0]->width;
               //TODO中间代码生成
            }
            else
            {
                T->width = 0;
                //TODO中间代码生成
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

        case ARRAY_DEF:
            break;

        case ARRAY_GROUP:
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
    if (err_num == 0)
        print_symbols();
}