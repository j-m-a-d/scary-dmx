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
    
    _odata.highThreshold = (dmx_value_t)[high intValue];
    _odata.lowThreshold = (dmx_value_t)[low intValue];
    _odata.speed = (unsigned int)[speed intValue];
    dmx_channel_t chs[] = {(dmx_channel_t)[channelStepper intValue],0};
    _odata.dmxChannels = channel_list_from_data(1, chs);
    [speedLevel setIntValue:[speed intValue]];
    
    [speedLevel setFrameRotation:90.0F];
}

- (IBAction)toggleOscillator:(id)sender
{
    if([control state] == NSOnState){
        start_oscillating(&_odata);
    } else {
        stop_oscillating();
    }
}

- (IBAction)updateHighThreshold:(id)sender
{
    _odata.highThreshold = (dmx_value_t)[high intValue];
}

- (IBAction)updateLowThreshold:(id)sender
{
    _odata.lowThreshold = (dmx_value_t)[low intValue];
}

- (IBAction)updateSpeed:(id)sender
{
    _odata.speed = (unsigned int)[speed intValue];
    [speedLevel setDoubleValue:[speed maxValue] - [speed doubleValue]];
}

-(IBAction)updateChannel:(id)sender
{
    _odata.dmxChannels->channels[0] = (dmx_channel_t)[channelStepper intValue];
    [channelField setIntValue:[channelStepper intValue]];
}

@end
