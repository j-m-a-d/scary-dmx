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

typedef int *channel_list_t;

static inline channel_list_t new_channel_list(size_t length)
{
    size_t __chan_legnth_init = sizeof(int) * (length + 1);
    channel_list_t v = malloc(__chan_legnth_init);
    memset(v, 0, __chan_legnth_init);
    return v;
}

//
#define COPY_CHANNEL_LIST(new, in, length)  \
        new_channel_list(length); \
        memcpy(new, in, (sizeof(int) * (length)) )
//

//

//
#endif
