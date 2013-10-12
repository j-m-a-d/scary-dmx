/*
 *  config_parser.h
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 10/16/12.
 *
 */

#ifndef Scary_DMX_config_parser_h
#define Scary_DMX_config_parser_h

extern void yyrestart(FILE*);
extern int yyparse();
extern int yylex();
extern int yyerror();
extern int parse_show_file(const char *filename, dmx_show_t **show);

#endif
