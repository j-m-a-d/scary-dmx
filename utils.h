/*
 *  utils.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 12/27/09.
 *  Copyright 2009 Inspirotech Inc. All rights reserved.
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char dmx_value_t;
typedef unsigned int dmx_channel_t;

struct channel_list {
    dmx_channel_t *channels;
    unsigned int length;
};

typedef struct channel_list *channel_list_t;

static inline channel_list_t new_channel_list(int length)
{
    size_t __chan_length_init = sizeof(dmx_channel_t) * (length + 1);
    channel_list_t v = malloc(sizeof(v));
    v->channels = malloc(__chan_length_init);
    memset(v->channels, 0, __chan_length_init);
    v->length = length;    
    return v;
}

static inline channel_list_t channel_list_from_data(int length, int *data)
{
    channel_list_t retval = new_channel_list(length);
    memcpy(retval->channels, data, sizeof(dmx_channel_t) * length);
    return retval;
}

static inline channel_list_t copy_channel_list(channel_list_t in)
{
    channel_list_t retval = new_channel_list(in->length);
    memcpy(retval->channels, in->channels, (sizeof(dmx_channel_t) * in->length) );
    return retval;
}

static inline void delete_channel_list(channel_list_t in)
{
    memset(in->channels, 0, sizeof(dmx_channel_t) * in->length);
    free(in->channels);
    in->channels = 0;
    memset(in, 0, sizeof(struct channel_list));
    free(in);
}

#define DELETE_CHANNEL_LIST(in) \
    if(in){ \
        delete_channel_list(in); \
    } \
    in = 0;


#ifdef _LOG
    #define log_info(format, ...) \
        fprintf (stdout, "[INFO]  "); \
        fprintf (stdout, format, ##__VA_ARGS__)

    #define log_debug(format, ...) \
        fprintf (stderr, "[DEBUG] [%s:%d] ", __FILE__, __LINE__); \
        fprintf (stderr, format, ##__VA_ARGS__)

    #define log_warn(format, ...) \
        fprintf (stderr, "[WARN]  " ); \
        fprintf (stderr, format, ##__VA_ARGS__)

    #define log_error(format, ...) \
        fprintf (stderr, "[ERROR] [%s:%d] ", __FILE__, __LINE__ ); \
        fprintf (stderr, format, ##__VA_ARGS__)
#elif
    #define log_info(format, ...)
    #define log_debug(format, ...)
    #define log_warn(format, ...)
    #define log_error(format, ...)
#endif

#endif

