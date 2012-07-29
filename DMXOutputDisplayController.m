/*
 *  DMXOutputDisplayController.m
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 11/14/11.
 *  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
 */

#import "DMXOutputDisplayController.h"
#include "dmx_controller.h"


@implementation DMXOutputDisplayController


- (void)awakeFromNib
{
    [analyzer start];
    timer = [NSTimer scheduledTimerWithTimeInterval:0.01 
                                             target:self 
                                           selector:@selector(updateBuffer) 
                                           userInfo:nil 
                                            repeats:YES];
    [timer retain];
}

- (void)updateBuffer
{
    dmx_value_t vdmx[MONITORED_DMX_CHANNELS];
    get_channel_buffer(vdmx, 1, MONITORED_DMX_CHANNELS);
    int i=0;
    for(i=0; i<MONITORED_DMX_CHANNELS; i++){
        buffer[i] =  (float)(vdmx[i]/255.0f);
    }
    [analyzer update:MONITORED_DMX_CHANNELS :buffer];   
}

- (void)windowWillClose:(NSNotification *)notification
{
    [super windowWillClose:notification];
    [timer invalidate];
}


-(void) dealloc
{
    [super dealloc];
    [timer release];
}

@end