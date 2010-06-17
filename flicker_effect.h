/*
 *  flicker_effect.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

enum {
    FLICKER_OK,
    FLICKER_IN_PROGRESS,
    FLICKER_BAD_CHANNEL
};
    
// Start the 'flicker' effect thread.
int start_flicker(int);

// Stop the 'flicker' effect thread.
void stop_flicker();

