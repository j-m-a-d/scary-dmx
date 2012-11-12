/*
 *  OscillatorController.m
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 11/15/09.
 *  Copyright 2009 Inspirotech Inc. All rights reserved.
 */

#import "OscillatorController.h"

@implementation OscillatorController

- (void)awakeFromNib
{
    
    odata.highThreshold = (dmx_value_t)[high intValue];
    odata.lowThreshold = (dmx_value_t)[low intValue];
    odata.speed = [speed intValue];
    unsigned int chs[] = {[channelStepper intValue],0};
    odata.dmxChannels = channel_list_from_data(1, chs);
    [speedLevel setIntValue:[speed intValue]];
    
    [speedLevel setFrameRotation:90.0F];
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
    odata.highThreshold = (dmx_value_t)[high intValue];
}

- (IBAction)updateLowThreshold:(id)sender
{
    odata.lowThreshold = (dmx_value_t)[low intValue];
}

- (IBAction)updateSpeed:(id)sender
{
    odata.speed = [speed intValue];
    [speedLevel setDoubleValue:[speed maxValue] - [speed doubleValue]];
}

-(IBAction)updateChannel:(id)sender
{
    odata.dmxChannels->channels[0] = [channelStepper intValue];
    [channelField setIntValue:[channelStepper intValue]];
}

@end
