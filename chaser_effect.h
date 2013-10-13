/*
 *  chaser_effect.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 12/20/09.
 *  Copyright 2009 Inspirotech Inc. All rights reserved.
 *
 */

#ifndef Scary_DMX_CHASE_EFFECT_H
#define Scary_DMX_CHASE_EFFECT_H

#include "utils.h"

typedef struct _chaser_step_t {
    channel_list_t channel;
    dmx_value_t value;
    dmx_speed_t speed;
    long pre_delay;
    long post_delay;
} chaser_step_t;

typedef struct _chaser_data_t {
    int data_steps;
    chaser_step_t *data;
    struct _chaser_data_t *next_step;
} chaser_data_t;

#endif

