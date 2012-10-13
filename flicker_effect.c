/*
 *  flicker_effect.c
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "flicker_effect.h"
#include "dmx_controller.h"
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


static pthread_t _flicker_thread = 0;
static pthread_mutex_t _flicker_mutex = PTHREAD_MUTEX_INITIALIZER;

static int flickering = 0;

/*
    Stop the flicker effect thread.
*/
void stop_flicker()
{
    pthread_mutex_lock(&_flicker_mutex);
        flickering = 0;
    pthread_mutex_unlock(&_flicker_mutex);
    cancel_join_pthread(&_flicker_thread, "flicker");
}

static void reset_dmx_state(void *data)
{
    channel_list_t dmxChannels = (channel_list_t)data;
    update_channels(dmxChannels, 0);
}

/*
	This thread will update channels like a chaser that 
    creates a flicker effect.
*/
void *flicker(void *channels)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    channel_list_t dmxChannels = (channel_list_t)channels;

    pthread_cleanup_push(reset_dmx_state, dmxChannels);
    
    /* Effect speed. */
	useconds_t seconds = 10000;
    
	register dmx_value_t i=0;
	while(1){
        /* Hardcoded sequence of values for this effect on a dimmer pack. */
	    for(i=65; i<155; i+=2){ 
            update_channels(dmxChannels, i); 
            usleep(seconds);
        }

        for(i=155; i>100; i-=2){ 
            update_channels(dmxChannels, i); 
            usleep(seconds);
        }

		for(i=100; i<125; i+=2){ 
            update_channels(dmxChannels, i); 
            usleep(seconds);
        }

		for(i=125; i>75; i-=2){ 
            update_channels(dmxChannels, i);
            usleep(seconds);
        }

		for(i=75; i<125; i+=2){ 
            update_channels(dmxChannels, i);
            usleep(seconds);
        }

		for(i=125; i>65; i-=2){ 
            update_channels(dmxChannels, i); 
            usleep(seconds);
        }
	}

    pthread_cleanup_pop(1);
	EXIT_THREAD();
}

/*
    Start the flicker effect thread on the specified channel.
*/
int start_flicker(channel_list_t dmxChannels){

    int status = FLICKER_OK;
    pthread_mutex_lock(&_flicker_mutex);
    if(flickering){
        pthread_mutex_unlock(&_flicker_mutex);
        return FLICKER_IN_PROGRESS;
    }else{
        flickering = 1;
        if(validate_channel_list(dmxChannels)) {
            //return -1;
        }
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
        //pthread_attr_setstacksize(&attr, 512);
        pthread_create(&_flicker_thread, &attr, flicker, (void *)(dmxChannels));
    }
    pthread_mutex_unlock(&_flicker_mutex);
    return status;
}
