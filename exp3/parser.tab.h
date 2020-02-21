
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     ID = 259,
     RELOP = 260,
     TYPE = 261,
     CHAR = 262,
     FLOAT = 263,
     DPLUS = 264,
     LP = 265,
     RP = 266,
     LC = 267,
     RC = 268,
     LT = 269,
     RT = 270,
     SEMI = 271,
     COMMA = 272,
     PLUS = 273,
     MINUS = 274,
     STAR = 275,
     DIV = 276,
     ASSIGNOP = 277,
     AND = 278,
     OR = 279,
     NOT = 280,
     IF = 281,
     ELSE = 282,
     WHILE = 283,
     RETURN = 284,
     FOR = 285,
     SWITCH = 286,
     CASE = 287,
     COLON = 288,
     DEFAULT = 289,
     CONTINUE = 290,
     BREAK = 291,
     INC = 292,
     DEC = 293,
     ADD_ASSIGN = 294,
     SUB_ASSIGN = 295,
     PROGRAM = 296,
     EXT_DEF_LIST = 297,
     EXT_VAR_DEF = 298,
     FUNC_DEF = 299,
     FUNC_DEC = 300,
     EXT_DEC_LIST = 301,
     PARAM_LIST = 302,
     PARAM_DEC = 303,
     VAR_DEF = 304,
     DEC_LIST = 305,
     DEF_LIST = 306,
     COMP_STM = 307,
     STM_LIST = 308,
     EXP_STMT = 309,
     IF_THEN = 310,
     IF_THEN_ELSE = 311,
     FUNC_CALL = 312,
     ARGS = 313,
     FUNCTION = 314,
     PARAM = 315,
     ARG = 316,
     CALL = 317,
     LABEL = 318,
     GOTO = 319,
     JLT = 320,
     JLE = 321,
     JGT = 322,
     JGE = 323,
     EQ = 324,
     NEQ = 325,
     ARRAY_DEC = 326,
     ARRAY_VIS = 327,
     ARRAY_DEF = 328,
     ARRAY_GROUP = 329,
     UMINUS = 330,
     LOWER_THEN_ELSE = 331
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 17 "parser.y"

	int    type_int;
	float  type_float;
	char   type_char;
	char   type_id[32];
	struct ASTNode *ptr;



/* Line 1676 of yacc.c  */
#line 138 "parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;

