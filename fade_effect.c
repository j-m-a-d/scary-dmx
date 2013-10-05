/*
 *  fade_effect.c
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 9/29/13.
 *
 */

#include "fade_effect.h"
#include "utils.h"
#include "dmx_controller.h"
#include "unistd.h"

typedef struct _fader_handle_t {
    pthread_t *handle;
} fader_handle_t;

void free_fader(fader_data_t *fader)
{
    FREE_CHANNEL_LIST(fader->channels);
    memset(fader, 0, sizeof(fader_data_t));
    free(fader);
}

void print_fader_data(const fader_data_t *fader, FILE *out)
{
    fprintf(out, "fader {\n");
    printChannelList(fader->channels, out);
    fprintf(out, "\t\t from:%d;\n", fader->from_value);
    fprintf(out, "\t\t to:%d;\n", fader->to_value);
    fprintf(out, "\t\t speed:%u\n", fader->speed);
    fprintf(out, "\t}\n");
}
/*
 * from current value to desired value
 * completes when all channels reach target value
 */
void fade_channels_to(const fader_data_t *fader)
{
    dmx_channel_t *ch = fader->channels->channels;
    register int r = 1;
    while(r) {
        r = 0;
        for(unsigned int i=0; i < fader->channels->length; i++, ch++){
            dmx_value_t v = get_channel_value(*ch);
            if( v < fader->to_value) {
                update_channel(*ch, ++v);
                r++;
            } else if( v > fader->to_value) {
                update_channel(*ch, --v);
                r++;
            }
        }
        usleep(fader->speed);
    }
}

/*
 * fade channel(s) from a specified value to a target specified value
 * completes when all channels reach target value.
 */
/* blocking */
void fade_channels(const fader_data_t *fader)
{
    update_channels(fader->channels, fader->from_value);
    fade_channels_to(fader);
}

static void *fader_function(void *data)
{
    fader_data_t *fader = (fader_data_t*)data;
    fade_channels(fader);
    if(fader->callback) {
        fader->callback(fader->callback_data);
    }
    EXIT_THREAD();
}

/* from current location to desired value (async complete when target value is reached) */
void fader_channels_async(const fader_data_t *fader)
{
    pthread_t thread;
    spawn_joinable_pthread(&thread, fader_function, (void*)fader);
}

/* continuous fade from current to target auto determines direction (async) until cancelled */
fid start_fader_for_channels(const fader_data_t *fader)
{
    return 0;
}

int cancel_fader(fid fader_handle_t)
{
    return 0;
}

int cancel_all_faders()
{
    return 0;
}
