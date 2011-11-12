//
//  AnalyzerDisplayHandler.m
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/8/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import "AnalyzerDisplayController.h"
#import "QuickTime/QuickTime.h"

@implementation AnalyzerDisplayController

void freqCallback(void* objRef, QTAudioFrequencyLevels* freq_levels)
{
    id myself = (AnalyzerDisplayController*)objRef;
    [myself updateAnalyzer:freq_levels->numFrequencyBands * 2 : freq_levels->level];
}

-(void)updateAnalyzer:(int)count: (float*)levels
{
    [analyzer update:count : levels];
}

- (void)awakeFromNib
{
    registerSelfAsFreqListener((void*)self, freqCallback);
    [analyzer start];
}

- (void)windowWillClose:(NSNotification *)notification
{
    deregisterSelfAsFreqListner((void*)self);
}

@end
