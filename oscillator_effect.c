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
volatile static int _oscillating = 0;

static pthread_t _oscillator_thread = 0;
static pthread_mutex_t _oscillator_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 Print an oscillator setting to a show file.
 */
void print_oscillator_data(const oscillator_data_t *data, FILE *out)
{
    fprintf(out, "\toscillator {\n");
    printChannelList(data->channels, out);
    fprintf(out, "\t\t low:%d;\n", data->lowThreshold);
    fprintf(out, "\t\t high:%d;\n", data->highThreshold);
    fprintf(out, "\t\t speed:%d;\n", data->speed);
    fprintf(out, "\t}\n");
}

inline void copy_oscillator_data(oscillator_data_t *to, const oscillator_data_t *from)
{
    memset(to, 0, ODT_SIZE);
    memcpy(to, from, ODT_SIZE);
}

inline void free_oscillator_data(oscillator_data_t *odata)
{
    if(odata){
        FREE_CHANNEL_LIST(odata->channels);
        memset(odata, 0, ODT_SIZE);
        free(odata);
        odata = 0;
    }
}

static void reset_dmx_state(void *data)
{
    channel_list_t dmxChannels = ((oscillator_data_t*)data)->channels ;
    update_channels(dmxChannels, CHANNEL_RESET);
}

/*
 * Oscillate a channel.
 */
void oscillate(const oscillator_data_t *odata)
{
    register dmx_value_t i=0;
    for(i=odata->lowThreshold; i< odata->highThreshold-1; i+=2){
        update_channels(odata->channels, i);
        usleep(odata->speed);
    }
    
    for(i=odata->highThreshold; i> odata->lowThreshold+1; i-=2){
        update_channels(odata->channels, i);
        usleep(odata->speed);
    }

}

/*
 *  This thread will update channels like a chaser that
 *  oscillates from a low threshold to a high threshold.
 */
static void *_oscillate(void* data_in)
{
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.oscillator");
    
    oscillator_data_t *val = (oscillator_data_t*)data_in;
  
    pthread_cleanup_push(reset_dmx_state, data_in);
    
    while(_oscillating)
        oscillate(val);
    
    pthread_cleanup_pop(1);
    EXIT_THREAD();
}

/*
    Stop the oscillating thread.
*/
void stop_oscillating()
{
    pthread_mutex_lock(&_oscillator_mutex);
        _oscillating=0;    
    pthread_mutex_unlock(&_oscillator_mutex);
    cancel_join_pthread(&_oscillator_thread);
}

/*
    Start the oscillator effect with the specified options.
*/
int start_oscillating(const oscillator_data_t *inData)
{
    /* signal we are ready to oscillate. */
    pthread_mutex_lock(&_oscillator_mutex);    
        if(_oscillating){
            pthread_mutex_unlock(&_oscillator_mutex);
            return OSCILLATOR_IN_PROGRESS;
        }
        _oscillating = 1;
        spawn_joinable_pthread(&_oscillator_thread, _oscillate, (void *)inData);
    pthread_mutex_unlock(&_oscillator_mutex);
    
    return OSCILLATOR_OK;
}
