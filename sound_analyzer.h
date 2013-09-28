/*
 *  soundanalyzer.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/15/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */
#ifndef _SOUND_ANALYZER_H
#define _SOUND_ANALYZER_H

#include <QuickTime/QuickTime.h>
#include "utils.h"

#define MAX_ANALYZE_CHANNELS 2

enum  {
    ANALYZE_OK                  =0 << 0,
    ANALYZE_IN_PROGRESS         =1 << 0,
    ANALYZE_FILE_NOT_FOUND      =1 << 1,
    ANALYZE_MOVIE_NOT_FOUND     =1 << 2,
    ANALYZE_MOVIE_CREATE_FAILED =1 << 3
};

enum {
    analyzeMonitorPeak,
    analyzeMonitorFollow,
    analyzeMonitorChase
};

typedef struct _analyzer_data_t {
    char *movieFile;
    channel_list_t dmxChannelList;
    dmx_value_t dmxValue;
    unsigned long numberOfBandLevels;
    short frequency;
    double threshold;
    int flags;
} analyzer_data_t;

#define NEW_ANALYZER_DATA_T(data) \
    malloc(sizeof(analyzer_data_t)); \
    memset(data, 0, sizeof(analyzer_data_t))

/*
 Print an analyzer setting to a show file.
 */
void print_analyzer(analyzer_data_t *, FILE *);
int open_movie_file(const unsigned char *fileName, Movie **newMovie, short *refId);
int start_analyze(analyzer_data_t *data_in, void(*callback)());
void stop_analyze();
void skip_movie();
void free_analyzer_data(analyzer_data_t *);
void register_self_as_freq_listener(void *callbackRef, void(*listenerFunction)(void*, QTAudioFrequencyLevels*));
void deregister_self_as_freqListner(void *callbackRef);

#define FREE_ANALYZER_DATA(data) \
free_analyzer_data(data); \
data = 0;

#endif
