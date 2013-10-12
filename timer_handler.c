/*
 *  timer.c
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 10/11/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "timer_handler.h"
#include "dmx_controller.h"
#include "effects_handle.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/time.h>

volatile static int _timed = 0;

static pthread_mutex_t _wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _wait_cond = PTHREAD_COND_INITIALIZER;

typedef struct _timer_t {
    pthread_t *thread_handle;
    volatile int run_flag;
} timer_t;

/*
 Print a timer setting to a show file.
 */
void print_timer_data(const timer_data_t *data, FILE *out)
{
    fprintf(out, "\ttimer {\n");
    fprintf(out, "\t\t ontime:%d;\n", data->on_time);
    fprintf(out, "\t\t offtime:%d;\n", data->off_time);
    print_effects_handle(data->effect, out);
    fprintf(out, "\t}\n");
}

static void free_timer_handle(timer_t *in_timer)
{
    timer_t *timer = in_timer;
    pthread_cancel( *timer->thread_handle);
    pthread_join( *timer->thread_handle, NULL);

    if(timer->thread_handle) {
        free(timer->thread_handle);
    }

    memset(timer, 0, sizeof(timer_t));
    free(in_timer);
}

static void free_timer(timer_data_t *data)
{
    if(data){
        /* If the show was never cued up, we'll have no allocations */
        if(data->timer_handle){
            free_timer_handle((timer_t*)data->timer_handle);
        }

        data->timer_handle = 0;

        FREE_EFFECTS_HANDLE(data->effect);
        memset(data, 0, sizeof(timer_data_t));
        free(data);
    }
}

void free_timers(timer_data_t *timer)
{
    if(timer) {
        free_timers(timer->nextTimer);
        free_timer(timer);
    }
}

int timers_init()
{
    int result = 0;

    pthread_mutex_lock(&_wait_mutex);
    result = pthread_cond_init(&_wait_cond, NULL);
    pthread_mutex_unlock(&_wait_mutex);

    return result;
}

void stop_timers(timer_data_t *timer)
{

    if(!timer) return;

    pthread_mutex_lock(&_wait_mutex);
    _timed = 0;

    timer_data_t *tmp = timer;
    while(tmp){
        timer_t* handle = ((timer_t*)(tmp->timer_handle));
        if(!handle) continue;
        cancel_join_pthread((handle->thread_handle));
        handle->run_flag = 0;
        reset_effects_channels(tmp->effect);
        tmp = tmp->nextTimer;
    }
    pthread_mutex_unlock(&_wait_mutex);
}

static inline uint64_t get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * (unsigned long)tv.tv_sec + (unsigned long)tv.tv_usec;
}

static inline uint8_t time_compare(const uint64_t then, dmx_speed_t limit)
{
    const uint64_t now = get_timestamp();
    return ((now - then) < limit ? 1 : 0);
}

static void *do_timer(void *data_in)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.timer");

    timer_data_t *data = (timer_data_t*)data_in;

    /* Wait here until the timers are told to start. */
    pthread_mutex_lock(&_wait_mutex);
    while(!_timed) {
        pthread_cond_wait(&_wait_cond, &_wait_mutex);
    }
    pthread_mutex_unlock(&_wait_mutex);

    while(1){

        uint64_t check = get_timestamp();
        while( time_compare(check, data->on_time)) {
            do_effect(data->effect);
            usleep(100);
        }

        reset_effects_channels(data->effect);
        usleep(data->off_time);
    }

    EXIT_THREAD();
}

int create_timer_handle(timer_handle **handle)
{
    timer_t *timer = 0;
    timer = malloc(sizeof(timer_t));
    if(!timer) return TIMED_EFFECT_FAIL; // combine

    timer->thread_handle = malloc(sizeof(pthread_t));
    memset(timer->thread_handle, 0, sizeof(pthread_t));
    timer->run_flag = 0;

    *handle = (timer_handle*)timer;

    return TIMED_EFFECT_OK;
}

int cue_timer(const timer_data_t *data)
{
    timer_t *te = (timer_t*)data->timer_handle;
    te->run_flag = 1;
    int result = spawn_joinable_pthread(te->thread_handle, do_timer, (void*)(data) );
    if(result){
        return result;
    }

    return TIMED_EFFECT_OK;
}

int start_timers()
{
    int result = 0;
    if(0 == pthread_mutex_lock(&_wait_mutex)){
        _timed = 1;
        result = pthread_cond_broadcast(&_wait_cond);
        pthread_mutex_unlock(&_wait_mutex);
    }
    return result;
}
