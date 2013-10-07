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

volatile static int _flickering = 0;

inline void free_flicker_data(flicker_data_t *flicker_data)
{
    if(flicker_data){
        FREE_CHANNEL_LIST(flicker_data->channels);
        free(flicker_data);
    }
}

/*
 Print a flicker value to a show file.
 */
void print_flicker_channels(const flicker_data_t *flicker_data, FILE *out)
{
    fprintf(out, "\tflicker {\n");
    printChannelList(flicker_data->channels, out);
    fprintf(out, "\t}\n");
}

/*
    Stop the flicker effect thread.
*/
void stop_flicker()
{
    pthread_mutex_lock(&_flicker_mutex);
        _flickering = 0;
    pthread_mutex_unlock(&_flicker_mutex);
    cancel_join_pthread(&_flicker_thread);
}

static void reset_dmx_state(void *data)
{
    channel_list_t dmxChannels = (channel_list_t)data;
    update_channels(dmxChannels, CHANNEL_RESET);
}

void flicker(const flicker_data_t *flicker)
{
    
    /* Effect speed. */
	useconds_t seconds = 10000;
    channel_list_t dmxChannels = flicker->channels;
    
    register dmx_value_t i=0;
    
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
/*
	This thread will update channels like a chaser that
    creates a flicker effect.
*/
static void *_flicker(void *fdata)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.flicker");
    
    flicker_data_t *flicker_data = (flicker_data_t*)fdata;

    pthread_cleanup_push(reset_dmx_state, flicker_data->channels);

    while(1) {
        flicker(flicker_data);
    }

    pthread_cleanup_pop(1);
	EXIT_THREAD();
}

/*
    Start the flicker effect thread on the specified channel.
*/
int start_flicker(const flicker_data_t *flicker_data){

    int status = FLICKER_OK;
    pthread_mutex_lock(&_flicker_mutex);
    if(_flickering){
        pthread_mutex_unlock(&_flicker_mutex);
        return FLICKER_IN_PROGRESS;
    }else{
        _flickering = 1;
        if(validate_channel_list(flicker_data->channels, DMX_CHANNELS)) {
            //return -1;
        }
        spawn_joinable_pthread(&_flicker_thread, _flicker, (void *)(flicker_data));
    }
    pthread_mutex_unlock(&_flicker_mutex);
    return status;
}
