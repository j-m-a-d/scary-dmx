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
//#define MAX_ANALYZE_BANDS 20

#define FREE_ANALYZER_DATA(data) \
    free_analyzer_data(data); \
    data = 0;

enum  {
    ANALYZE_OK,
    ANALYZE_IN_PROGRESS,
    ANALYZE_FILE_NOT_FOUND,
    ANALYZE_MOVIE_NOT_FOUND,
    ANALYZE_MOVIE_CREATE_FAILED
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
    float threshold;
    int flags;
} analyzer_data_t;

#define NEW_ANALYZER_DATA_T(data) \
    malloc(sizeof(analyzer_data_t)); \
    memset(data, 0, sizeof(analyzer_data_t))

int open_movie_file(const unsigned char *fileName, Movie **newMovie, short *refId);

int start_analyze(analyzer_data_t *data_in, void(*callback)());

void stop_analyze();

void skip_movie();

void free_analyzer_data(analyzer_data_t *);

void registerSelfAsFreqListener(void *callbackRef, void(*listenerFunction)(void*, QTAudioFrequencyLevels*));
void deregisterSelfAsFreqListner(void *callbackRef);

#endif
