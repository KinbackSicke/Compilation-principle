echo
flex lex.l
bison -d -v parser.y
gcc -o TAC lex.yy.c parser.tab.c ast.c semantic_IR.c objectcode.c
pause