//
//  OscillatorController.m
//  Scary DMX
//
//  Created by Jason DiPrinzio on 11/15/09.
//  Copyright 2009 Inspirotech Inc. All rights reserved.
//

#import "OscillatorController.h"

@implementation OscillatorController

- (void)awakeFromNib
{
    odata.channel = [channelStepper intValue];
    odata.highThreshold = [high intValue];
    odata.lowThreshold = [low intValue];
    odata.speed = [speed intValue];
    [speedLevel setIntValue:[speed intValue]];
    
    [speedLevel setFrameRotation:90.0];
}

- (IBAction)toggleOscillator:(id)sender
{
    if([control state] == NSOnState){
        start_oscillating(&odata);
    } else {
        stop_oscillating();
    }
}

- (IBAction)updateHighThreshold:(id)sender
{
    odata.highThreshold = [high intValue];
}

- (IBAction)updateLowThreshold:(id)sender
{
    odata.lowThreshold = [low intValue];
}

- (IBAction)updateSpeed:(id)sender
{
    odata.speed = [speed intValue];
    [speedLevel setIntValue:[speed maxValue] - [speed intValue]];
}

-(IBAction)updateChannel:(id)sender
{
    odata.channel = [channelStepper intValue];
    [channelField setIntValue:[channelStepper intValue]];
}

@end
