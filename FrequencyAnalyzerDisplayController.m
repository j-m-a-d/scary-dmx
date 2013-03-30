/*
 *  AnalyzerDisplayHandler.m
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 11/8/11.
 *  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
 */

#import "FrequencyAnalyzerDisplayController.h"
#import "sound_analyzer.h"
#import "QuickTime/QuickTime.h"

@implementation FrequencyAnalyzerDisplayController

void freqCallback(void* objRef, QTAudioFrequencyLevels* freq_levels)
{
    id myself = (FrequencyAnalyzerDisplayController*)objRef;
    [myself updateAnalyzer:freq_levels->numFrequencyBands * 2 : freq_levels->level];
}

- (void)_start
{
    register_self_as_freq_listener((void*)self, freqCallback);
}

- (IBAction)show:(id)sender
{
    [super show:sender];
    [self _start];
}

- (void)awakeFromNib
{
    [self _start];
    [analyzer start];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [super windowWillClose:notification];
    deregister_self_as_freqListner((void*)self);
}

- (void)updateAnalyzer:(int)count :(float*)levels
{
    [analyzer update:(unsigned int)count : levels];
}

@end
