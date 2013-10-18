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
    channel_list_t channels;
    dmx_value_t value;
    uint16_t repeat;
    dmx_speed_t speed;
    dmx_speed_t pre_delay;
    dmx_speed_t post_delay;
} chaser_step_t;

typedef struct _chaser_t {
    chaser_step_t *step;
    struct _chaser_t *next_step;
} chaser_t;

#define NEW_CHASER_STEP(chaser) \
    malloc(sizeof(chaser_step_t)); \
    memset(chaser, 0, sizeof(chaser_step_t));

void free_chaser_step(chaser_step_t *);
#define FREE_CHASER_STEP(chaser) \
    free_chaser_step(chaser); \
    chaser = 0;

#define NEW_CHASER(chaser) \
    malloc(sizeof(chaser_t)); \
    memset(chaser, 0, sizeof(chaser_t));

void free_chaser(chaser_t*);
#define FREE_CHASER(chaser) \
    free_chaser(chaser); \
    chaser = 0;

void print_chaser_data(const chaser_t*, FILE*);
void add_chaser_step(chaser_t*, chaser_step_t*);
void chase(const chaser_t*);

#endif
