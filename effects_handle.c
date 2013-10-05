//
//  effects_handle.c
//  Scary DMX
//
//  Created by Jason Diprinzio on 10/3/13.
//
//

#include <stdio.h>
#include "effects_handle.h"
#include "flicker_effect.h"
#include "oscillator_effect.h"

void new_effects_handle()
{
    effects_handle_t *effect;
    effect = malloc(sizeof(effects_handle_t));
}

inline void do_effect(effects_handle_t *effect)
{
    switch(effect->type) {
        case effect_type_fader:
            fade_channels_to(effect->effect.fader);
            break;
        case effect_type_flicker:
            flicker(effect->effect.flicker);
            break;
        case effect_type_oscillator:
            oscillate(effect->effect.oscillator);
            break;
        default:
            break;
    }
}
