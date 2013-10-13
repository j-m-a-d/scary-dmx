#include <stdio.h>
#include "show_handler.h"


int main(int argc, char **argv)
{
    dmx_show_t *resultShow;

    int i = init_show(&resultShow);
    if(i){
        log_error("Failed to initialize show.\n");
        return i;
    }

    if( (i =load_show_from_file(argv[1], &resultShow )) ){
        log_debug("Show not found or invalid show.\n");
        goto quit;
    }

    i =0;
    printShow(resultShow, stdout);

quit:
    FREE_SHOW (resultShow);
    return i;
}

