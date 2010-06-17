/*
 *  chaser_handler.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 12/20/09.
 *  Copyright 2009 Inspirotech Inc. All rights reserved.
 *
 */


typedef struct _chaser_step_t {
    unsigned short channel;
    unsigned char value;
    unsigned char _pad;
    long pre_delay;
    long post_delay;
} chaser_step_t;

typedef struct _chaser_data_t {
    int data_steps;
    chaser_step_t *data;
    struct _chaser_data_t *next_step;
} chaser_data_t;
