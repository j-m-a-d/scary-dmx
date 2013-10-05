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
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

volatile static int _timed = 0;

static pthread_mutex_t _wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _wait_cond = PTHREAD_COND_INITIALIZER;

typedef struct _timed_effect_t {
    pthread_t *thread_handle;
    volatile int run_flag;
} timed_effect_t;

/*
 Print a timer setting to a show file.
 */
void print_timer_data(const timed_effect_data_t *data, FILE *showFile)
{
    fprintf(showFile, "\ttimer {\n");
    printChannelList(data->channels, showFile);
    fprintf(showFile, "\t\t ontime:%d;\n", data->on_time);
    fprintf(showFile, "\t\t offtime:%d;\n", data->off_time);
    fprintf(showFile, "\t\t onvalue:%d;\n", data->on_value);
    fprintf(showFile, "\t\t offvalue:%d;\n", data->off_value);
    fprintf(showFile, "\t}\n");
}

static void free_timer_handle(timed_effect_t *in_timer)
{
    timed_effect_t *timer = in_timer;
    pthread_cancel( *timer->thread_handle);
    pthread_join( *timer->thread_handle, NULL);
    
    if(timer->thread_handle) {
        free(timer->thread_handle);
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
        
        FREE_CHANNEL_LIST(data->channels);
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
    _timed = 0;
    
    timed_effect_data_t *tmp = timer;
    while(tmp){
        timed_effect_t* handle = ((timed_effect_t*)(tmp->timer_handle));
        if(!handle) continue;
        cancel_join_pthread((handle->thread_handle));
        handle->run_flag = 0;
        update_channels(tmp->channels, CHANNEL_RESET);
        tmp = tmp->nextTimer;
    }
    pthread_mutex_unlock(&_wait_mutex);
}

static void *do_timed_effect(void *data_in)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.timer");

    timed_effect_data_t *data = (timed_effect_data_t*)data_in;

    /* Wait here until the timers are told to start. */
    pthread_mutex_lock(&_wait_mutex);
    while(!_timed) {
        pthread_cond_wait(&_wait_cond, &_wait_mutex);
    }
    pthread_mutex_unlock(&_wait_mutex);

    while(1){
        update_channels(data->channels, data->on_value);
        usleep(data->on_time);
        update_channels(data->channels, data->off_value);
        usleep(data->off_time);
    }
    
    EXIT_THREAD();
}

int create_timed_effect_handle(timed_effect_handle **handle)
{
    timed_effect_t *timer = 0;
    timer = malloc(sizeof(timed_effect_t));
    if(!timer) return TIMED_EFFECT_FAIL; // combine

    timer->thread_handle = malloc(sizeof(pthread_t));
    memset(timer->thread_handle, 0, sizeof(pthread_t));
    timer->run_flag = 0;
    
    *handle = (timed_effect_handle*)timer;
    
    return TIMED_EFFECT_OK;
}

int cue_timed_effect(timed_effect_data_t *data)
{   
    timed_effect_t *te = (timed_effect_t*)data->timer_handle;
    te->run_flag = 1;
    int result = spawn_joinable_pthread(te->thread_handle, do_timed_effect, (void*)(data) );
    if(result){
        return result;
    }
    
    return TIMED_EFFECT_OK;
}

int start_timed_effects()
{
    int result = 0;
    if(0 == pthread_mutex_lock(&_wait_mutex)){
        _timed = 1;
        result = pthread_cond_broadcast(&_wait_cond);
        pthread_mutex_unlock(&_wait_mutex);
    }
    return result;
}
