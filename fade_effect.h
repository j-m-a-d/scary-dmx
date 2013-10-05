/*
 *  fade_effect.h
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 9/29/13.
 *
 */

#ifndef Scary_DMX_fade_effect_h
#define Scary_DMX_fade_effect_h

#include "utils.h"

/* function type for fader callbacks */
typedef int(*fader_complete_callback)(void*);

/* identifier for fader processes */
typedef struct fader_t *fid;

typedef struct _fader_data_t {
    channel_list_t channels;
    dmx_value_t from_value;
    dmx_value_t to_value;
    dmx_speed_t speed;
    fader_complete_callback callback;
    void *callback_data;
} fader_data_t;

typedef fader_data_t *fader_t;

#define NEW_FADER_DATA(fader) \
    malloc(sizeof(fader_data_t)); \
    memset(fader, 0, sizeof(fader_data_t));

void free_fader(fader_data_t *);
#define FREE_FADER_DATA(fader) \
    free_fader(fader); \
    fader = 0;

/*
 * print a fader data structure to a stream
 */
void print_fader_data(const fader_data_t *, FILE*);

/*
 * fade channel(s) from a specified value to a target specified value
 * completes when all channels reach target value.
 */
/* blocking */
void fade_channels(const fader_data_t *);
/* from current value (last known value) to desired value */
void fade_channels_to(const fader_data_t *);

/* from current location to desired value (async complete when target value is reached) */
void fader_channels_async(const fader_data_t *);

/* continuous fade from current to target auto determines direction (async) until cancelled */
fid start_fader_for_channels(const fader_data_t*);
int cancel_fader(const fid);
int cancel_all_faders();

#endif
