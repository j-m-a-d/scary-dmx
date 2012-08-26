/*
 *  timed_effect.c
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 10/11/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "timed_effect.h"
#include "dmx_controller.h"
#include "unistd.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

volatile static int timed = 0;

static pthread_mutex_t _wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _wait_cond = PTHREAD_COND_INITIALIZER;

typedef struct _timed_effect_t {
    pthread_t *handle;
    volatile int run_flag;
} timed_effect_t;

static void free_timer_handle(timed_effect_t *in_timer)
{
    timed_effect_t *timer = (timed_effect_t *)in_timer;
    pthread_cancel( *timer->handle);
    pthread_join( *timer->handle, NULL);
    
    if(timer->handle) {
        free(timer->handle);
    }
    
    memset(timer, 0, sizeof(timed_effect_t));
    free(in_timer);
}

static void free_timed_effect(timed_effect_data_t *data)
{
    if(data){
        /* If the show was never cued up, we'll have no allocations */
        if(data->timer_handle){
            free_timer_handle((timed_effect_t*)data->timer_handle);
        }
        
        data->timer_handle = 0;
        
        delete_channel_list(data->channels);
        data->channels = 0;
        
        memset(data, 0, sizeof(timed_effect_data_t));
        free(data);
    }
}

void free_timed_effects(timed_effect_data_t *timer)
{
    if(timer) {
        free_timed_effects(timer->nextTimer);
        free_timed_effect(timer);
    }
}

int timed_effects_init()
{    
    int result = 0;
    
    pthread_mutex_lock(&_wait_mutex);
    result = pthread_cond_init(&_wait_cond, NULL);
    pthread_mutex_unlock(&_wait_mutex);
    
    return result;
}

void stop_timed_effects(timed_effect_data_t *timer)
{
    
    if(!timer) return;

    pthread_mutex_lock(&_wait_mutex);
    timed = 0;
    
    timed_effect_data_t *tmp = timer;
    while(tmp){
        timed_effect_t* handle = ((timed_effect_t*)tmp->timer_handle);
        if(!handle) continue;
        if(ESRCH == pthread_cancel(*handle->handle)) {
            fprintf(stderr, "WARNING: Cannot cancel Timer; Thread not found.\n");
        }
        handle->run_flag = 0;
        update_channels(tmp->channels, 0);
        tmp = tmp->nextTimer;
    }
    pthread_mutex_unlock(&_wait_mutex);
}

void *do_timed_effect(void *data_in)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    timed_effect_data_t *data = (timed_effect_data_t*)data_in;
    /* Wait here until the timers are told to start. */
    pthread_mutex_lock(&_wait_mutex);
    while(!timed) {
        pthread_cond_wait(&_wait_cond, &_wait_mutex);
    }
    pthread_mutex_unlock(&_wait_mutex);

    while(1){
        update_channels(data->channels, data->value);
        usleep(data->on_time);
        update_channels(data->channels, 0 );
        usleep(data->off_time);
    }
    pthread_exit(NULL);
}

int create_timed_effect_handle(timed_effect_handle **handle)
{
    timed_effect_t *timer = 0;
    timer = malloc(sizeof(timed_effect_t));
    if(!timer) return TIMED_EFFECT_FAIL; // combine

    timer->handle = malloc(sizeof(pthread_t));
    memset(timer->handle, 0, sizeof(pthread_t));
    timer->run_flag = 0;
    
    *handle = (timed_effect_handle*)timer;
    
    return TIMED_EFFECT_OK;
}

int cue_timed_effect(timed_effect_data_t *data)
{   
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 512);

    timed_effect_t *te = (timed_effect_t*)data->timer_handle;
    te->run_flag = 1;
    int result = pthread_create(te->handle, &attr, do_timed_effect, (void*)(data) );
    /* Check result */
    if(result){}
    
    return TIMED_EFFECT_OK;
}

int start_timed_effects()
{
    int result = 0;
    if(0 == pthread_mutex_lock(&_wait_mutex)){
        timed = 1;
        result = pthread_cond_broadcast(&_wait_cond);
        pthread_mutex_unlock(&_wait_mutex);
    }
    return result;
}
