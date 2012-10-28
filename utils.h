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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>


#define DMX_CHANNELS            513

#ifdef _LOG
#include <libgen.h>
#define log_info(format, ...) \
fprintf (stdout, "[INFO]  "); \
fprintf (stdout, format, ##__VA_ARGS__)

#define log_debug(format, ...) \
fprintf (stderr, "[DEBUG] [%s:%d] ", basename(__FILE__), __LINE__); \
fprintf (stderr, format, ##__VA_ARGS__)

#define log_warn(format, ...) \
fprintf (stderr, "[WARN]  " ); \
fprintf (stderr, format, ##__VA_ARGS__)

#define log_error(format, ...) \
fprintf (stderr, "[ERROR] [%s:%d] ", basename(__FILE__), __LINE__ ); \
fprintf (stderr, format, ##__VA_ARGS__)
#else
#define log_info(format, ...)
#define log_debug(format, ...)
#define log_warn(format, ...)
#define log_error(format, ...)
#endif


typedef unsigned char dmx_value_t;
typedef unsigned int dmx_channel_t;

enum channel_ret {
    CHANNEL_LIST_OK,
    CHANNEL_LIST_BAD_CHANNEL
};

struct channel_list {
    dmx_channel_t *channels;
    unsigned int length;
};

typedef struct channel_list *channel_list_t;

/*
 Print a list of cue channels from a show to a file.
 */
void printCueChannels(unsigned char *, FILE *);

/*
 Print a list of channels for any effect that supports channes lists.
 */
void printChannelList(channel_list_t, FILE *);

channel_list_t new_channel_list(const unsigned int);
channel_list_t channel_list_from_data(const unsigned int, const unsigned int *);
channel_list_t copy_channel_list(const channel_list_t);
void delete_channel_list(const channel_list_t);
#define DELETE_CHANNEL_LIST(in) \
    if(in){ \
        delete_channel_list(in); \
    } \
    in = 0;

int validate_channel_list(const channel_list_t, const unsigned int);

#define THREAD_FINISHED 0
static const unsigned int _THREAD_FINISHED = 11544216;

#define EXIT_THREAD() \
    pthread_exit((void*)&_THREAD_FINISHED); 

int spawn_joinable_pthread(pthread_t *thread, void*(*func)(void*), void *data);
int cancel_join_pthread(const pthread_t*, const char *name);

#endif

