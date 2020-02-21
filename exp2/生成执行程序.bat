echo
flex lex.l
bison -d -v parser.y
gcc -o symbol lex.yy.c parser.tab.c ast.c semantic.c
pause