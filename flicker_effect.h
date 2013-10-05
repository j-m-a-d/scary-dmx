/*
 *  flicker_effect.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#ifndef Scary_DMX_FLICKER_EFFECT_H
#define Scary_DMX_FLICKER_EFFECT_H

#include "utils.h"

enum {
    FLICKER_OK,
    FLICKER_IN_PROGRESS,
    FLICKER_BAD_CHANNEL
};

typedef struct _flicker_data_t {
    channel_list_t flicker_channels;
} flicker_data_t;

#define NEW_FLICKER_DATA(flicker_data) \
    malloc(sizeof (flicker_data_t)); \
    memset(flicker_data, 0, sizeof(flicker_data_t));

void free_flicker_data(flicker_data_t *);
#define FREE_FLICKER_DATA(flicker_data) \
    free_flicker_data(flicker_data); \
    flicker_data = 0;

/*
 Print a flicker value to a show file.
 */
void print_flicker_channels(const flicker_data_t *, FILE *);

/* make some lights flicker */
void flicker(const flicker_data_t *);
/* Start the 'flicker' effect thread. */
int start_flicker(const flicker_data_t*);

/* Stop the 'flicker' effect thread. */
void stop_flicker();

#endif
