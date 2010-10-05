/*
 *  oscillator_effect.h
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/18/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */
 
#define ODT_SIZE sizeof(oscillator_data_t)

#define NEW_OSCILLATOR_DATA_T(data) \
        malloc(ODT_SIZE); \
        memset(data, 0, ODT_SIZE);

#define FREE_OSCILLATOR_DATA(data) \
        free_oscillator_data(data); \
        data = 0;

enum {
    OSCILLATOR_OK,
    OSCILLATOR_IN_PROGRESS,
    OSCILLATOR_NOT_RUNNING,
    OSCILLATOR_BAD_CHANNEL
};

//
typedef struct _oscillator_data_t{
    int channel;
    int lowThreshold;
    int highThreshold;
    int speed;
} oscillator_data_t;
//

inline void copy_oscillator_data(oscillator_data_t *, const oscillator_data_t *);
inline void free_oscillator_data(oscillator_data_t *);

// Start the oscillating effect thread on a channel.
int start_oscillating(const oscillator_data_t *);

// Stop the oscillating effect thread.
void stop_oscillating();
