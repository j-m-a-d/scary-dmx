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

struct _channel_list_t {
    int *channels;
    int length;
};

typedef struct _channel_list_t *channel_list_t;
//    
static inline channel_list_t new_channel_list(int length)
{
    size_t __chan_length_init = sizeof(int) * (length + 1);
    channel_list_t v = malloc(sizeof(struct _channel_list_t));
    v->channels = malloc(__chan_length_init);
    memset(v->channels, 0, __chan_length_init);
    v->length = length;    
    return v;
}
//
static inline channel_list_t channel_list_from_data(int length, int *data)
{
    channel_list_t retval = new_channel_list(length);
    memcpy(retval->channels, data, sizeof(int) * length);
    return retval;
}
//
static inline channel_list_t copy_channel_list(channel_list_t in)
{
    channel_list_t retval = new_channel_list(in->length);
    memcpy(retval->channels, in->channels, (sizeof(int) * in->length) );
    return retval;
}
//
static inline void delete_channel_list(channel_list_t in)
{
    memset(in->channels, 0, sizeof(int) * in->length);
    free(in->channels);
    in->channels = 0;
    memset(in, 0, sizeof(struct _channel_list_t));
    free(in);
}
//
#define DELETE_CHANNEL_LIST(in) \
    if(in){ \
        delete_channel_list(in); \
    } \
    in = 0;
//
#endif
