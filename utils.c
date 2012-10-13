//
//  utils.c
//  Scary DMX
//
//  Created by Jason Diprinzio on 10/13/12.
//
//

#include "utils.h"
#include "dmx_controller.h"

int cancel_join_pthread(pthread_t *thread, const char* name)
{
    if(!thread || !*thread) {
        return 0;
    }
    
    if(ESRCH == pthread_cancel(*thread)) {
        unsigned int *retval[1];
        int result = pthread_join(*thread, (void**)&(retval[0]));
        if(!result) {
            if(_THREAD_FINISHED != *retval[0]) {
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

inline channel_list_t new_channel_list(int length)
{
    size_t __chan_length_init = sizeof(dmx_channel_t) * (length + 1);
    channel_list_t v = malloc(sizeof(v));
    v->channels = malloc(__chan_length_init);
    memset(v->channels, 0, __chan_length_init);
    v->length = length;
    return v;
}

inline channel_list_t channel_list_from_data(int length, int *data)
{
    channel_list_t retval = new_channel_list(length);
    memcpy(retval->channels, data, sizeof(dmx_channel_t) * length);
    return retval;
}

inline channel_list_t copy_channel_list(channel_list_t in)
{
    channel_list_t retval = new_channel_list(in->length);
    memcpy(retval->channels, in->channels, (sizeof(dmx_channel_t) * in->length) );
    return retval;
}

inline void delete_channel_list(channel_list_t in)
{
    memset(in->channels, 0, sizeof(dmx_channel_t) * in->length);
    free(in->channels);
    in->channels = 0;
    memset(in, 0, sizeof(struct channel_list));
    free(in);
}

inline int validate_channel_list(channel_list_t in)
{
    dmx_channel_t *tmp = in->channels;
    while(*tmp++){
        if( *tmp >= DMX_CHANNELS ){
            return CHANNEL_LIST_BAD_CHANNEL;
        }
    }
    return CHANNEL_LIST_OK;
}
