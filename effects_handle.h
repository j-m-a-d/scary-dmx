//
//  effects_handler.h
//  Scary DMX
//
//  Created by Jason Diprinzio on 10/3/13.
//
//

#ifndef Scary_DMX_effects_handler_h
#define Scary_DMX_effects_handler_h

#include "oscillator_effect.h"
#include "flicker_effect.h"
#include "fade_effect.h"
#include "chaser_effect.h"

typedef enum {
    effect_type_onoff,
    effect_type_oscillator,
    effect_type_flicker,
    effect_type_fader,
    effect_type_chaser,
} effect_type;

typedef struct _onoff_effect_t {
    uint8_t flag;
    channel_list_t channels;
    dmx_value_t on_value;
    dmx_value_t off_value;
} onoff_effect_t;

#define NEW_ONOFF_EFFECT(effect) \
    malloc(sizeof(onoff_effect_t)); \
    memset(effect, 0, sizeof(onoff_effect_t)); \
    effect->flag = 1;

void free_onoff_effect(onoff_effect_t *);
#define FREE_ONOFF_EFFECT(effect) \
    free_onoff_effect(effect); \
    effect = 0;

typedef struct _effects_handle_t {
    effect_type type;
    union _effects_handler_t {
        onoff_effect_t *onoff;
        oscillator_data_t *oscillator;
        flicker_data_t *flicker;
        fader_data_t *fader;
        chaser_t *chaser;
    } effect;
} effects_handle_t;

void print_effects_handle(const effects_handle_t*, FILE*);

#define NEW_EFFECTS_HANDLE(handle) \
    malloc(sizeof(effects_handle_t)); \
    memset(handle, 0, sizeof(effects_handle_t));

void free_effects_handle(effects_handle_t*);
#define FREE_EFFECTS_HANDLE(handle) \
    free_effects_handle(handle); \
    handle = 0;

void do_effect(const effects_handle_t*);
void reset_effects_channels(const effects_handle_t*);

#endif
