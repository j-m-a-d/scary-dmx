/*
 *  timer.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 10/11/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#ifndef Scary_DMX__TIMED_EFFECT_H
#define Scary_DMX__TIMED_EFFECT_H

#include "utils.h"
#include "effects_handle.h"

typedef struct timer_t *timer_handle;

typedef struct _timer_data_t {
    effects_handle_t *effect;
    dmx_time_t on_time;
    dmx_time_t off_time;
    timer_handle *timer_handle;
    struct _timer_data_t *next_timer;
} timer_data_t;

#define NEW_TIMER(timer) \
    malloc(sizeof(timer_data_t)); \
    memset(timer, 0, sizeof(timer_data_t));

enum {
    TIMED_EFFECT_OK,
    TIMED_EFFECT_FAIL,
    TIMED_EFFECT_IN_PROGRESS
};

/*
 Print a timer setting to a show file.
 */
void print_timer_data(const timer_data_t *, FILE *);
int timers_init();
int create_timer_handle(timer_handle**);
int cue_timer(const timer_data_t*);
int start_timers();
void stop_timers(timer_data_t*);

#define FREE_TIMED_EFFECTS(data) \
if(data) { \
    free_timers(data); \
    data = 0; \
}

void free_timers(timer_data_t *);

#endif
