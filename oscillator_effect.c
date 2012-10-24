/*
 *  oscillator_effect.c
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "oscillator_effect.h"
#include "dmx_controller.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Flag that indicates this thread is running. */
volatile static int oscillating = 0;

static pthread_t _oscillator_thread = 0;
static pthread_mutex_t oscillator_mutex = PTHREAD_MUTEX_INITIALIZER;

inline void copy_oscillator_data(oscillator_data_t *to, const oscillator_data_t *from)
{
    memset(to, 0, ODT_SIZE);
    memcpy(to, from, ODT_SIZE);
}

inline void free_oscillator_data(oscillator_data_t *tdata)
{
    if(tdata){
        DELETE_CHANNEL_LIST(tdata->dmxChannels);
        memset(tdata, 0, ODT_SIZE);
        free(tdata);
        tdata = 0;
    }
}

static void reset_dmx_state(void *data)
{
    channel_list_t dmxChannels = ((oscillator_data_t*)data)->dmxChannels ;
    update_channels(dmxChannels, CHANNEL_RESET);
}

/*
 *  This thread will update channels like a chaser that
 *  oscillates from a low threshold to a high threshold.
 */
void *oscillate(void* data_in)
{
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    
    oscillator_data_t *val = (oscillator_data_t*)data_in;
  
    pthread_cleanup_push(reset_dmx_state, data_in);
    
    register dmx_value_t i=0;
    while(oscillating){
        for(i=val->lowThreshold; i<val->highThreshold-1; i+=2){ 
            update_channels(val->dmxChannels, i);
            usleep(val->speed);
        }

        for(i=val->highThreshold; i>val->lowThreshold+1; i-=2){ 
            update_channels(val->dmxChannels, i);
            usleep(val->speed);
        }        
    }
    
    pthread_cleanup_pop(1);
    EXIT_THREAD();
}

/*
    Stop the oscillating thread.
*/
void stop_oscillating()
{
    pthread_mutex_lock(&oscillator_mutex);
        oscillating=0;    
    pthread_mutex_unlock(&oscillator_mutex);
    cancel_join_pthread(&_oscillator_thread, "oscillator");
}

/*
    Start the oscillator effect with the specified options.
*/
int start_oscillating(const oscillator_data_t *inData)
{   
    /*
     if(inData->channel > DMX_CHANNELS ){
        return OSCILLATOR_BAD_CHANNEL;
    }*/
    
    /* signal we are ready to oscillate. */
    pthread_mutex_lock(&oscillator_mutex);    
        if(oscillating){
            pthread_mutex_unlock(&oscillator_mutex);
            return OSCILLATOR_IN_PROGRESS;
        }
        oscillating = 1;
        spawn_joinable_pthread(&_oscillator_thread, oscillate, (void *)inData);
    pthread_mutex_unlock(&oscillator_mutex);
    
    return OSCILLATOR_OK;
}    
    
    
