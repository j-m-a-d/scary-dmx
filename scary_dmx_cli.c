//
//  scary_dmx_cli.c
//  Scary DMX
//
//  Created by Jason Diprinzio on 10/16/12.
//
//

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <QuickTime/QuickTime.h>
#include "show_handler.h"

void sig_all(int sig)
{
    sigset_t mask_set;
    sigset_t old_set;
    
    signal(sig, sig_all);
    sigfillset(&mask_set);
    sigprocmask(SIG_SETMASK, &mask_set, &old_set);
    
	stop_show();
	free_all_show();
	destroy_dmx();
	ExitMovies();
	log_info("Shutdown finished.\n");
	exit(0);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        log_error("No show file supplied\n");
        return 1;
    }
    
    (void)signal(SIGINT, &sig_all);
    (void)signal(SIGQUIT, &sig_all);
    (void)signal(SIGKILL, &sig_all);
    
    EnterMovies();
    
    dmx_show_t *newShow;
    char *showFile;
    showFile = argv[1];
    if(!showFile){
        log_debug("Usage: %s showfile.\n", argv[0]);
        return 2;
    }
    if(load_show_from_file(showFile, &newShow )){
        log_debug("Show not found or invalid show.\n");
        return 3;
    }
    
    if(DMX_INIT_OK != init_dmx()){
        log_debug("Failed to open DMX device.\n");
        return 4;
    }
    
    start_dmx();
    log_error( "Starting show.\n");
    start_show();
    log_error( "Running...\n");
    while(1){
        //usleep(100000);
        sleep(5);
    }
    
    return 0;
}

#ifdef _EXT_PARSER
int main(int argc, char **argv)
{
    extern FILE *yyin;
    yyin = fopen(argv[1], "r");
    
    dmx_show_t *resultShow;
    
    int i = init_show(&resultShow);
    if(i){
        log_error("Failed to initialize show.\n");
    }
    i = yyparse();
    if(i){
        log_error("Parse error.\n");
        return i;
    }
    _rewind_show(resultShow);
    
    FILE *outFile = fopen("./outshow.shw", "w+");
    if(!outFile){
        log_error("Could not open out file for show output.\n");
    }
    printShow(resultShow, outFile);
    fclose(outFile);
    
    FREE_SHOW (resultShow);
    return i;
}
#endif


