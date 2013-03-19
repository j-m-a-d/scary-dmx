/*
 *  flicker_effect.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#ifndef _FLICKER_EFFECT_H
#define _FLICKER_EFFECT_H

#include "utils.h"

enum {
    FLICKER_OK,
    FLICKER_IN_PROGRESS,
    FLICKER_BAD_CHANNEL
};

/*
 Print a flicker value to a show file.
 */
void print_flicker_channels(channel_list_t, FILE *);

/* Start the 'flicker' effect thread. */
int start_flicker(channel_list_t);

/* Stop the 'flicker' effect thread. */
void stop_flicker();

#endif
