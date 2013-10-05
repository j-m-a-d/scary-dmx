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
#include "timed_effect.h"

typedef enum {
    effect_type_oscillator,
    effect_type_flicker,
    effect_type_fader,
} effect_type;

typedef struct _off_on_effect {
    dmx_value_t on_value;
    dmx_value_t off_value;
} off_on_effect_t;

typedef struct _effects_handle_t {
    effect_type type;
    union _effects_handler_t {
        oscillator_data_t *oscillator;
        flicker_data_t *flicker;
        fader_data_t *fader;
        off_on_effect_t *off_on;
    } effect;
} effects_handle_t;

#endif
