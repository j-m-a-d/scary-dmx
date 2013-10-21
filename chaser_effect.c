/*
 *  chaser_effect.c
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 12/20/09.
 *  Copyright 2009 Inspirotech Inc. All rights reserved.
 *
 */

#include "chaser_effect.h"
#include "dmx_controller.h"
#include <unistd.h>

static void print_chaser_step(chaser_step_t *step, FILE *out)
{
    fprintf(out, "\t step {\n");
    print_channel_list(step->channels, out);
    fprintf(out, "\t\t chase_value: %d;\n", step->value);
    fprintf(out, "\t\t repeat: %d;\n", step->repeat);
    fprintf(out, "\t\t speed: %u;\n", step->speed);
    fprintf(out, "\t\t pre_delay: %u;\n", step->pre_delay);
    fprintf(out, "\t\t post_delay: %u;\n", step->post_delay);
    fprintf(out, "\t }\n");
}

void print_chaser_data(const chaser_t *chaser, FILE *out)
{
    fprintf(out, "\t chaser {\n");
    const chaser_t *tmp = chaser;
    while(tmp){
        print_chaser_step(chaser->step, out);
        tmp = tmp->next_step;
    }
    fprintf(out, "\t }\n");
}

void free_chaser_step(chaser_step_t *step)
{
    FREE_CHANNEL_LIST(step->channels);
    memset(step, 0, sizeof(chaser_step_t));
    free(step);
}

void free_chaser(chaser_t *chaser)
{
    while(chaser) {
        chaser_t *tmp = chaser;
        chaser = chaser->next_step;
        FREE_CHASER_STEP(tmp->step);
        memset(tmp, 0, sizeof(chaser_t));
        free(tmp);
    }
}

void add_chaser_step(chaser_t *chaser, chaser_step_t *step)
{
    if(!chaser->step) {
        chaser->step = step;
    } else {
        chaser_t *tmp = chaser;
        while(tmp) {
            chaser = tmp;
            tmp = tmp->next_step;
        }
        chaser->next_step = NEW_CHASER(chaser->next_step);
        chaser->next_step->step = step;
    }
}

void chase(const chaser_t *chaser)
{
    while(chaser) {
        chaser_step_t *step = chaser->step;
        usleep(step->pre_delay);
        for(register int i=0; i<step->repeat; i++) {
            dmx_channel_t *ch = step->channels->channels;
            for(register uint16_t j=0; j < step->channels->length; j++) {
                update_channel(*ch, step->value);
                usleep(step->speed);
                update_channel(*ch, 0);
                ch++;
            }
        }
        usleep(step->post_delay);
        chaser = chaser->next_step;
    }
}
