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

#ifndef _REENTRANT
#error Compile with _REENTRANT defined for use with threads
#endif

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
    char errbuf[128];

    if(signal(SIGINT, &sig_all)== SIG_ERR){
        strerror_r(errno, errbuf, 128);
        log_error("Failed to install handler for SIGINT - %s\n", errbuf);
    }
    if(signal(SIGQUIT, &sig_all) == SIG_ERR){
        strerror_r(errno, errbuf, 128);
        log_error("Failed to install handler for SIGQUIT - %s\n", errbuf);
    }
    
    if(argc < 2) {
        log_error("No show file supplied\n");
        return 1;
    }
    
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
        // TODO create keyboard shortcuts and handle them.
        sleep(1);
    }
    
    return 0;
}
