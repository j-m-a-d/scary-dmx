/*
 *  timed_effect.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 10/11/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#define FREE_TIMED_EFFECTS(data) \
    free_timed_effects(data); \
    data = 0;

typedef struct timed_effect_t *timed_effect_handle;

typedef struct _timed_effect_data_t {
    int channel;
    short value;
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
int create_timed_effect_handle(timed_effect_data_t *data, timed_effect_handle *);
void free_timed_effect(timed_effect_data_t *);