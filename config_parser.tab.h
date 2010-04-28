/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
     VALUE = 258,
     CHANNEL = 259,
     CHANNEL_LIST = 260,
     FLOAT_VALUE = 261,
     FILE_SPEC = 262,
     ERROR = 263,
     CUE = 264,
     CHAN = 265,
     FLICKER = 266,
     OSCILLATOR = 267,
     ANALYZER = 268,
     TIMER = 269,
     SPEED = 270,
     LOW = 271,
     HIGH = 272,
     FILENAME = 273,
     TYPE = 274,
     FREQ = 275,
     THRESHOLD = 276,
     BANDS = 277,
     THRESHOLD_VALUE = 278,
     DMX_VALUE = 279,
     ONTIME = 280,
     OFFTIME = 281,
     LPAREN = 282,
     RPAREN = 283,
     LBRACE = 284,
     RBRACE = 285,
     SEMICOLON = 286,
     DASH = 287,
     UNKNOWN = 288
   };
#endif
/* Tokens.  */
#define VALUE 258
#define CHANNEL 259
#define CHANNEL_LIST 260
#define FLOAT_VALUE 261
#define FILE_SPEC 262
#define ERROR 263
#define CUE 264
#define CHAN 265
#define FLICKER 266
#define OSCILLATOR 267
#define ANALYZER 268
#define TIMER 269
#define SPEED 270
#define LOW 271
#define HIGH 272
#define FILENAME 273
#define TYPE 274
#define FREQ 275
#define THRESHOLD 276
#define BANDS 277
#define THRESHOLD_VALUE 278
#define DMX_VALUE 279
#define ONTIME 280
#define OFFTIME 281
#define LPAREN 282
#define RPAREN 283
#define LBRACE 284
#define RBRACE 285
#define SEMICOLON 286
#define DASH 287
#define UNKNOWN 288




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 125 "config_parser.y"
{
    int     val;
    double  dval;
    char    *text;
    struct {
        int count;
        int channels[512];
    } chan_list;
}
/* Line 1529 of yacc.c.  */
#line 125 "config_parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

