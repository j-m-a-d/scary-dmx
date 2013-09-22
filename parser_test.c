#include <stdio.h>
#include "show_handler.h"

extern void yyrestart(FILE*);
extern int yyparse();
extern int yylex();

int parse_show_file(const char *filename, dmx_show_t **show)
{
    return 0;
}

int main(int argc, char **argv)
{
    extern FILE *yyin;
    yyin = fopen(argv[1], "r");

    dmx_show_t *resultShow;

    int i = init_show(&resultShow);
    if(i){
        log_error("Failed to initialize show.\n");
        return i;
    }

    yyrestart(yyin);
    i = yyparse();
    fclose(yyin);

    if(i){
        log_error("Parse error.\n");
        return i;
    }

    FILE *outFile = fopen("./outshow.shw", "w+");
    if(!outFile){
        log_error("Could not open out file for show output.\n");
    }
    printShow(resultShow, outFile);
    fclose(outFile);

    FREE_SHOW (resultShow);
    return i;
}

