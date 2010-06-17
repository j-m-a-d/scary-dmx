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


#define CHECK_OSCILLATING \
pthread_mutex_lock(&oscillator_mutex); \
if(!oscillating) { \
    pthread_mutex_unlock(&oscillator_mutex);\
        break; \
} \
pthread_mutex_unlock(&oscillator_mutex);
//
//  Flag that indicates this thread is running.
static int oscillating = 0;
//
static pthread_t oscillator_pt = 0;
static pthread_mutex_t oscillator_mutex = PTHREAD_MUTEX_INITIALIZER;

inline void copy_oscillator_data(oscillator_data_t *to, const oscillator_data_t *from)
{
    memset(to, 0, ODT_SIZE);
    memcpy(to, from, ODT_SIZE);
}

inline void free_oscillator_data(oscillator_data_t *tdata)
{
    if(tdata){
        memset(tdata, 0, ODT_SIZE);
        free(tdata);
        tdata = 0;
    }
}
/*
 *  This thread will update channels like a chaser that
 *  oscillates from a low threshold to a high threshold.
 */
void *Oscillate(void* data_in){
    
    oscillator_data_t *val = (oscillator_data_t*)data_in;
  
    int i=0;
    while(oscillating){
        for(i=val->lowThreshold; i<val->highThreshold; i+=2){ 
            update_channel(val->channel, i); 
            CHECK_OSCILLATING
            usleep(val->speed);
        }
        
        CHECK_OSCILLATING
        
        for(i=val->highThreshold; i>val->lowThreshold; i-=2){ 
            update_channel(val->channel, i);
            CHECK_OSCILLATING
            usleep(val->speed);
        }        
    }
    
cleanup:
    //Reset
    update_channel(val->channel, 0);
    pthread_exit(NULL);
}

/*
    Stop the oscillating thread.
*/
void stop_oscillating()
{
    pthread_mutex_lock(&oscillator_mutex);
        oscillating=0;    
    pthread_mutex_unlock(&oscillator_mutex);
    if(oscillator_pt)
        pthread_join(oscillator_pt, NULL);
    oscillator_pt = NULL;
}

/*
    Start the oscillator effect with the specified options.
*/
int start_oscillating(const oscillator_data_t *inData)
{   
    if(inData->channel > DMX_CHANNELS ){
        return OSCILLATOR_BAD_CHANNEL;
    }
    
    //signal we are ready to oscillate.
    pthread_mutex_lock(&oscillator_mutex);    
        if(oscillating){
            pthread_mutex_unlock(&oscillator_mutex);
            return OSCILLATOR_IN_PROGRESS;
        }
        oscillating = 1;
        pthread_create(&oscillator_pt, NULL, Oscillate, (void *)inData);
    pthread_mutex_unlock(&oscillator_mutex);
    
    return OSCILLATOR_OK;
}    
    
    
