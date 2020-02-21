#include "def.h"

#define REG_NUM 23
#define PARAM_NUM 4

REG regList[REG_NUM];
FILE *fp;

//初始化寄存器
void initRegList()
{
    int i;
    //$t0-$t9
    for (i = 0; i < 10; i++)
    {
        sprintf(regList[i].name, "$t%d", i);
        regList[i].flag = 0;
    }
    //$s0-$s8
    for (i = 0; i < 9; i++)
    {
        sprintf(regList[i + 10].name, "$s%d", i);
        regList[i].flag = 0;
    }
    //$a0-$a4 只分配给函数参数
    for (i = 0; i < PARAM_NUM; i++)
    {
        sprintf(regList[i + 19].name, "$a%d", i);
        regList[i].flag = 0;
    }
    
}

//分配寄存器
int allocateReg(char *name)
{
    int i;
    for (i = 0; i < REG_NUM - PARAM_NUM; i++)
    {
        if (regList[i].flag == 0)
        {
            regList[i].flag = 1;
            strcpy(regList[i].username, name);
            return i;
        }       
    }
    return overflow();
}

//检查后面代码是否要用到该变量
int check(CODE_NODE *head, CODE_NODE * current, char *name)
{
    CODE_NODE *p = current;
    if (name[0] == 'v')
        return 0;
    
    for(p = p->next; p != head; p = p->next)
    {
        if (strcmp(p->opn1.id, name) == 0)
            return 0;
        if (strcmp(p->opn2.id, name) == 0)
            return 0;
        if (strcmp(p->result.id, name) == 0)
            return 0;       
    }
    return 1;
}


int ensure(char * name)
{
    int i;
    for (i = 0; i < REG_NUM; i++)   
        if (regList[i].flag == 1 && strcmp(regList[i].username, name) == 0)
            return i;    
    return allocateReg(name);
}

//释放寄存器
void freeReg(int index)
{
    regList[index].flag = 0;
    strcpy(regList[index].username, "");
}

//溢出处理
int overflow()
{
    //随机释放一个寄存器
    srand((unsigned)time(NULL));
    int id = rand() % 19;
    fprintf(fp, "sw %s, %d($v1)", regList[id].name, id * 4);
    freeReg(id);
    return id;
}

void genObjectCode(CODE_NODE * head)
{
    initRegList();
    fp = fopen("code.asm", "wb+");
    CODE_NODE *h = head;
    int id1, id2, id3;

    fprintf(fp, "%s\n", ".data");
    fprintf(fp, "%s\n", "_prompt: .asciiz \"Enter an integer:\"");
    fprintf(fp, "%s\n", "_ret: .asciiz \"\\n\"");
    fprintf(fp, "%s\n", "_copy: .space 76");
    fprintf(fp, "%s\n", ".globl main0");
    fprintf(fp, "%s\n", ".text");
    
    //main0
    fprintf(fp, "\n%s\n", "main0:");
    //$v1用来保存溢出地址
    fprintf(fp, "\tla $v1, _copy\n");
    fprintf(fp, "\t%s\n", "addi $sp, $sp, -4");
    fprintf(fp, "\t%s\n", "sw $ra, 0");
    fprintf(fp, "\t%s\n", "jal main");
    fprintf(fp, "\t%s\n", "li $v0, 10");
    fprintf(fp, "\t%s\n", "syscall");

    //read func
    fprintf(fp, "\n%s\n", "read:");
    fprintf(fp, "\t%s\n", "li $v0, 4");
    fprintf(fp, "\t%s\n", "la $a0, _prompt");
    fprintf(fp, "\t%s\n", "syscall");
    fprintf(fp, "\t%s\n", "li $v0, 5");
    fprintf(fp, "\t%s\n", "syscall");
    fprintf(fp, "\t%s\n", "jr $ra");

    //write func
    fprintf(fp, "\n%s\n", "write:");
    fprintf(fp, "\t%s\n", "li $v0, 1");
    fprintf(fp, "\t%s\n", "syscall");
    fprintf(fp, "\t%s\n", "li $v0, 4");
    fprintf(fp, "\t%s\n", "la $a0, _ret");
    fprintf(fp, "\t%s\n", "syscall");
    fprintf(fp, "\t%s\n", "move $v0, $0");
    fprintf(fp, "\t%s\n", "jr $ra");

    do
    {
        if (h->op == ASSIGNOP)
        {
            if (h->opn1.kind == INT)
            {
                id3 = ensure(h->result.id);
                fprintf(fp, "\tli %s, %d\n", regList[id3].name, h->opn1.const_int);
                if (check(head, h, h->result.id))
                    freeReg(id3);              
            }
            else
            {
                id1 = ensure(h->opn1.id);
                id3 = ensure(h->result.id);
                fprintf(fp, "\tmove %s, %s\n", regList[id3].name, regList[id1].name);
                if (check(head, h, h->opn1.id))
                    freeReg(id1);
                if (check(head, h, h->result.id))
                    freeReg(id3);
            }           
        }
        else if (h->op == GOTO)
        {
            fprintf(fp, "\tj %s\n", h->result.id);
        }
        else if (h->op == LABEL)
        {
            fprintf(fp, "%s:\n", h->result.id);
        }
        else if (h->op == PLUS || h->op == MINUS || h->op == STAR || h->op == DIV)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            id3 = allocateReg(h->result.id);
            if (h->op == PLUS)
            {
                fprintf(fp, "\tadd %s, %s, %s\n", 
                    regList[id3].name, regList[id1].name, regList[id2].name);
            }
            else if (h->op == MINUS)
            {
                fprintf(fp, "\tsub %s, %s, %s\n", 
                    regList[id3].name, regList[id1].name, regList[id2].name);
            }
            else if (h->op == STAR)
            {
                fprintf(fp, "\tmul %s, %s, %s\n", 
                    regList[id3].name, regList[id1].name, regList[id2].name);
            }
            else if (h->op == DIV)
            {
                fprintf(fp, "\tdiv %s, %s\n", 
                    regList[id3].name, regList[id1].name, regList[id2].name);
                fprintf(fp, "\tmflo %s\n", regList[id3].name);
            }
            //释放不再需要的寄存器
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);

        }
        else if (h->op == EQ)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            fprintf(fp, "\tbeq %s, %s, %s\n", regList[id1].name, regList[id2].name, h->result.id);
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);           
        }
        else if (h->op == NEQ)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            fprintf(fp, "\tbne %s, %s, %s\n", regList[id1].name, regList[id2].name, h->result.id);
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);
        }
        else if (h->op == JGE)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            fprintf(fp, "\tbge %s, %s, %s\n", regList[id1].name, regList[id2].name, h->result.id);
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);
        }
        else if (h->op == JGT)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            fprintf(fp, "\tbgt %s, %s, %s\n", regList[id1].name, regList[id2].name, h->result.id);
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);
        }
        else if (h->op == JLE)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            fprintf(fp, "\tble %s, %s, %s\n", regList[id1].name, regList[id2].name, h->result.id);
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);
        }
        else if (h->op == JLT)
        {
            id1 = ensure(h->opn1.id);
            id2 = ensure(h->opn2.id);
            fprintf(fp, "\tblt %s, %s, %s\n", regList[id1].name, regList[id2].name, h->result.id);
            if (check(head, h, h->opn1.id))
                freeReg(id1);
            if (check(head, h, h->opn2.id))
                freeReg(id2);
        }
        else if (h->op == RETURN)
        {
            id3 = ensure(h->result.id);
            fprintf(fp, "\tmove $v0, %s\n", regList[id3].name);
            fprintf(fp, "\tjr $ra\n");
            if(check(head, h, h->result.id))
                freeReg(id3);
        }
        else if (h->op == FUNCTION)
        {
            fprintf(fp, "\n%s:\n", h->result.id);
            //处理函数参数
            int rtn = searchSymbolTable(h->result.id);
            int count = symbolTable.symbols[rtn].paramnum;
            printf("param num: %d\n", count);
            int i;
            CODE_NODE *p = h->next;
            for (i = 0; i < count; i++)
            {
                //把参数变量保存在寄存器$a0-$a4中
                regList[i + 19].flag = 1;
                strcpy(regList[i + 19].username, p->result.id);
                //printf("%s   %s  %s\n", regList[i].name, regList[i].username, p->result.id);
                p = p->next;
            }          
        }
        else if (h->op == CALL)
        {
            if (strcmp(h->opn1.id, "read") == 0)
            {
                fprintf(fp, "\taddi $sp, $sp, -4\n");
                fprintf(fp, "\tsw $ra, 0($sp)\n");
                fprintf(fp, "\tjal read\n");
                fprintf(fp, "\tlw $ra, 0($sp)\n");
                fprintf(fp, "\taddi $sp, $sp, 4\n");
                id2 = ensure(h->opn2.id);
                fprintf(fp, "\tmove %s, $v0\n", regList[id2].name);
                if(check(head, h, h->opn2.id))
                    freeReg(id2);
            }
            else if (strcmp(h->opn1.id, "write") == 0)
            {
                id3 = ensure(h->result.id);            
                fprintf(fp, "\taddi $sp, $sp, -8\n");
                fprintf(fp, "\tsw $ra, 0($sp)\n");
                fprintf(fp, "\tsw $a0, 4($sp)\n");
                fprintf(fp, "\tmove $a0, %s\n", regList[id3].name);
                fprintf(fp, "\tjal write\n");
                fprintf(fp, "\tlw $ra, 0($sp)\n");
                fprintf(fp, "\tlw $a0, 4($sp)\n");
                fprintf(fp, "\taddi $sp, $sp, 8\n");
                if (check(head, h, h->result.id))
                    freeReg(id3);
            }
            else
            {
                int rtn = searchSymbolTable(h->opn1.id);
                int count = symbolTable.symbols[rtn].paramnum;
                int i;
                CODE_NODE *p = h;
                for (i = 0; i < count; i++)
                    p = p->prior;
                fprintf(fp, "\taddi $sp, $sp, -%d\n", count * 4 + 4);
                fprintf(fp, "\tsw, $ra, 0($sp)\n");
                for (i = 0; i < count; i++)
                {
                    //处理ARG
                    id3 = ensure(p->result.id);
                    fprintf(fp, "\tsw, $a%d, %d($sp)\n", i, (i + 1) * 4);
                    fprintf(fp, "\tmove $a%d, %s\n", i, regList[id3].name);
                    if (check(head, p, p->result.id))
                        freeReg(id3);    
                    p = p->next;                
                }
                fprintf(fp, "\tjal %s\n", h->opn1.id);
                fprintf(fp, "\tlw $ra, 0($sp)\n");
                for (i = 0; i < count; i++)
                {
                    fprintf(fp, "\tlw $a%d, %d($sp)\n", i, (i + 1) * 4);
                }
                fprintf(fp, "\taddi, $sp, $sp, %d\n", count * 4 + 4);
                //获取返回值，这里规定所有函数都返回一个int值
                id3 = ensure(h->result.id);
                fprintf(fp, "\tmove %s, $v0\n",regList[id3].name);
            }           
        }
        h = h->next;
    } while (h != head);
}