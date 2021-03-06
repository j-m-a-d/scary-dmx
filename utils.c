/*
*  utils.c
*  Scary DMX
*
*  Created by Jason Diprinzio on 10/13/12.
*  Copyright 2008 Inspirotech Inc. All rights reserved.
*
*/

#include "utils.h"

/*
 * Print a list of cue channels from a show to a file.
 */
void print_cue_channels(unsigned char *channels, FILE *out)
{
    unsigned char *cv = channels;
    register unsigned int i;
    for(i=0; i< DMX_CHANNELS+1; i++){
        if(*cv){
            fprintf(out, "\tch%d:%d;\n", i, (int)(*cv));
        }
        cv++;
    }
}

/*
 * Print a list of channels for any effect that supports channes lists.
 */
void print_channel_list(const channel_list_t channels, FILE *out)
{
    dmx_channel_t *tmp = channels->channels;
    fprintf(out, "\t\t ch:%d", *tmp++);
    while(*tmp){
        fprintf(out, ",%d", *tmp);
        tmp++;
    }
    fprintf(out, ";\n");
}

inline channel_list_t new_channel_list(const uint32_t length)
{
    size_t __chan_length_init = sizeof(dmx_channel_t) * (length + 1);
    channel_list_t v = malloc(sizeof(struct channel_list));
    v->channels = malloc(__chan_length_init);
    memset(v->channels, 0, __chan_length_init);
    v->length = length;
    return v;
}

inline channel_list_t channel_list_from_data(const uint32_t length, const uint32_t *data)
{
    channel_list_t retval = new_channel_list(length);
    memcpy(retval->channels, data, sizeof(dmx_channel_t) * length);
    return retval;
}

inline channel_list_t copy_channel_list(const channel_list_t in)
{
    channel_list_t retval = new_channel_list(in->length);
    memcpy(retval->channels, in->channels, (sizeof(dmx_channel_t) * in->length) );
    return retval;
}

inline void free_channel_list(const channel_list_t in)
{
    memset(in->channels, 0, sizeof(dmx_channel_t) * in->length);
    free(in->channels);
    in->channels = 0;
    memset(in, 0, sizeof(struct channel_list));
    free(in);
}

int validate_channel_list(const channel_list_t in, const uint32_t max)
{
    dmx_channel_t *tmp = in->channels;
    while(*tmp++){
        if( *tmp >= max ){
            return CHANNEL_LIST_BAD_CHANNEL;
        }
    }
    return CHANNEL_LIST_OK;
}

inline int spawn_joinable_pthread(pthread_t *thread, void*(*func)(void*), void *data)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&attr, 512);
    int result = pthread_create(thread, &attr, func, (void*)data);
    pthread_attr_destroy(&attr);
    return result;
}

int cancel_join_pthread(const pthread_t *thread)
{
    if(!thread || !*thread) {
        return 0;
    }

    char name[64];
    /* Call beefore cancel or we could lose the thread and name. */
    if(pthread_getname_np(*thread, name, sizeof(name) / sizeof(char))) {
        strncpy(name,"undefined", 64);
    }

    log_debug("Cancelling thread '%s'.\n", name);

    if(ESRCH == pthread_cancel(*thread)) {
        int *retval[1];
        int result = pthread_join(*thread, (void**)&(retval[0]));
        if(!result) {
            if( PTHREAD_CANCELED != (void*)(retval[0]) && _THREAD_FINISHED != *retval[0]) {
                log_warn("Cannot cancel '%s'.  Thread exited with unknown value: %d.\n", name, *retval[0]);
                return *retval[0];
            }
            return THREAD_FINISHED;
        } else {
            char *error_code;
            switch(result) {
                case EDEADLK:
                    error_code = "EDEADLK";
                    break;
                case EINVAL:
                    error_code = "EINVAL";
                    break;
                case ESRCH:
                    error_code = "ERSCH";
                    break;
                default:
                    error_code = "UNKNOWN";
                    break;
            }
            log_warn("Failed to cancel '%s'; Could not join thread: [%s]\n", name, error_code);
            return result;
        }
    }
    return THREAD_FINISHED;
}
