/*
 *  timed_effect.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 10/11/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#ifndef _TIMED_EFFECT_H
#define _TIMED_EFFECT_H

#include "utils.h"

typedef struct timed_effect_t *timed_effect_handle;

typedef struct _timed_effect_data_t {
    int channel;
    dmx_value_t value;
    long on_time;
    long off_time;
    timed_effect_handle *timer_handle;
    struct _timed_effect_data_t *nextTimer;
} timed_effect_data_t;

enum {
    TIMED_EFFECT_OK,
    TIMED_EFFECT_FAIL,
    TIMED_EFFECT_IN_PROGRESS
};

int timed_effects_init();

int create_timed_effect_handle(timed_effect_handle **handle);

int cue_timed_effect(timed_effect_data_t *timer_data);

void free_timed_effect(timed_effect_data_t *);

int start_timed_effects();
void stop_timed_effects();

#define FREE_TIMED_EFFECTS(data) \
free_timed_effect(data); \
data = 0;

#endif
