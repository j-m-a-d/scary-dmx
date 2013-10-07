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
#include "dmx_controller.h"

void new_effects_handle()
{
    effects_handle_t *effect;
    effect = malloc(sizeof(effects_handle_t));
}

inline void free_onoff_effect(onoff_effect_t *effect)
{
    FREE_CHANNEL_LIST(effect->channels);
    free(effect);
}

void free_effects_handle(effects_handle_t *effect)
{
    if(effect) {
        switch(effect->type) {
            case effect_type_onoff:
                FREE_ONOFF_EFFECT(effect->effect.onoff);
                break;
            case effect_type_fader:
                FREE_FADER_DATA(effect->effect.fader);
                break;
            case effect_type_flicker:
                FREE_FLICKER_DATA(effect->effect.flicker);
                break;
            case effect_type_oscillator:
                FREE_OSCILLATOR_DATA(effect->effect.oscillator);
                break;
            default:
                break;
        }
        memset(effect, 0, sizeof(effects_handle_t));
        free(effect);
    }
}

void print_onoff_effect(onoff_effect_t *effect, FILE *out)
{
    fprintf(out, "onoff {\n");
    printChannelList(effect->channels, out);
    fprintf(out, "\t\t %d\n;", effect->flag);
    fprintf(out, "\t\t %d\n;", effect->on_value);
    fprintf(out, "\t\t %d\n;", effect->off_value);
    fprintf(out, "}");
    
}

void print_effects_handle(const effects_handle_t *effect, FILE *out)
{
    switch(effect->type) {
        case effect_type_onoff:
            print_onoff_effect(effect->effect.onoff, out);
            break;
        case effect_type_fader:
            print_fader_data(effect->effect.fader, out);
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

void do_onoff_effect(onoff_effect_t *effect)
{
    if(effect->flag) {
        update_channels(effect->channels, effect->on_value);
    } else {
        update_channels(effect->channels, effect->off_value);
    }
}

inline void do_effect(const effects_handle_t *effect)
{
    switch(effect->type) {
        case effect_type_onoff:
            do_onoff_effect(effect->effect.onoff);
            break;
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

inline static channel_list_t get_effects_channels(const effects_handle_t *effect)
{
    switch(effect->type) {
        case effect_type_onoff:
            return effect->effect.onoff->channels;
        case effect_type_fader:
            return effect->effect.fader->channels;
        case effect_type_flicker:
            return effect->effect.flicker->channels;
        case effect_type_oscillator:
            return effect->effect.oscillator->channels;
        default:
            break;
    }
}

inline void reset_effects_channels(const effects_handle_t *effect)
{
    dmx_value_t reset_value = CHANNEL_RESET;
    if(effect_type_onoff == effect->type) {
        reset_value = effect->effect.onoff->off_value;
    }
    update_channels(get_effects_channels(effect), reset_value);
}
