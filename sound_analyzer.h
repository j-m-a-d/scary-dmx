/*
 *  soundanalyzer.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/15/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#include <QuickTime/QuickTime.h>
#include "utils.h"

#define MAX_ANALYZE_CHANNELS 2
#define MAX_ANALYZE_BANDS 10

//
enum  {
    ANALYZE_OK,
    ANALYZE_IN_PROGRESS,
    ANALYZE_FILE_NOT_FOUND,
    ANALYZE_MOVIE_NOT_FOUND,
    ANALYZE_MOVIE_CREATE_FAILED
};

enum {
    analyzeMonitorPeak         = 0 << 0,
    analyzeMonitorFollow       = 1 << 0,
    analyzeSingleChannel       = 0 << 1,
    analyzeMultiChannel        = 1 << 1
};
 
//
typedef struct _analyzer_data_t {
    char *movieFile;
    channel_list_t dmxChannelList;
    short dmxValue;
    unsigned long numberOfBandLevels;
    short frequency;
    float threshold;
    int flags;
} analyzer_data_t;
//
#define NEW_ANALYZER_DATA_T(data) \
malloc(sizeof(analyzer_data_t)); \
memset(data, 0, sizeof(analyzer_data_t))
//

#ifndef pascal
#define pascal
#endif
//
int start_analyze(analyzer_data_t *data_in, void(*callback)() __attribute__((pascal)));
//
void stop_analyze();
//
void free_analyzer_data(analyzer_data_t *);
