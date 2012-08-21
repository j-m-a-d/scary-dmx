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


static pthread_t flicker_pt = 0;
static pthread_mutex_t flicker_mutex = PTHREAD_MUTEX_INITIALIZER;

static int flickering = 0;

/*
    Stop the flicker effect thread.
*/
void stop_flicker()
{
    pthread_mutex_lock(&flicker_mutex);
        flickering = 0;
    pthread_mutex_unlock(&flicker_mutex);
    pthread_cancel(flicker_pt);
    pthread_join(flicker_pt, NULL);
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
void *Flicker(void *channels)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    channel_list_t dmxChannels = (channel_list_t)channels;

    pthread_cleanup_push(reset_dmx_state, dmxChannels);
    
    /* Effect speed. */
	useconds_t seconds = 10000;
    
	register dmx_value_t i=0;
	while(flickering){
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
	pthread_exit(NULL);
}

/*
    Start the flicker effect thread on the specified channel.
*/
int start_flicker(channel_list_t dmxChannels){

    int status = FLICKER_OK;
    pthread_mutex_lock(&flicker_mutex);
    if(flickering){
        pthread_mutex_unlock(&flicker_mutex);
        return FLICKER_IN_PROGRESS;
    }else{
        flickering = 1;
        dmx_channel_t *tmp = dmxChannels->channels;
        while(*tmp++){
            if( *tmp >= DMX_CHANNELS ){
                flickering = 0;
                return FLICKER_BAD_CHANNEL;
            }
        }
        pthread_create(&flicker_pt, NULL, Flicker, (void *)(dmxChannels));
    }
    pthread_mutex_unlock(&flicker_mutex);
    return status;
}
