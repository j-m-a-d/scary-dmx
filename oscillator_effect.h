/*
 *  oscillator_effect.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#ifndef Scary_DMX_OSCILLATOR_EFFECT_H
#define Scary_DMX_OSCILLATOR_EFFECT_H

#include "utils.h"

#define ODT_SIZE sizeof(oscillator_data_t)

#define NEW_OSCILLATOR_DATA_T(data) \
        malloc(ODT_SIZE); \
        memset(data, 0, ODT_SIZE);

#define FREE_OSCILLATOR_DATA(data) \
        free_oscillator_data(data); \
        data = 0;

enum {
    OSCILLATOR_OK,
    OSCILLATOR_IN_PROGRESS,
    OSCILLATOR_NOT_RUNNING,
    OSCILLATOR_BAD_CHANNEL
};

typedef struct _oscillator_data_t{
    channel_list_t channels;
    dmx_value_t lowThreshold;
    dmx_value_t highThreshold;
    dmx_speed_t speed;
} oscillator_data_t;

void copy_oscillator_data(oscillator_data_t *, const oscillator_data_t *);
void free_oscillator_data(oscillator_data_t *);

/*
 Print an oscillator setting to a show file.
 */
void print_oscillator_data(const oscillator_data_t *, FILE *);

/* Oscillate a channel */
void oscillate(const oscillator_data_t *);
/* Start the oscillating effect thread on a channel. */
int start_oscillating(const oscillator_data_t *);
/* Stop the oscillating effect thread. */
void stop_oscillating();

#endif
