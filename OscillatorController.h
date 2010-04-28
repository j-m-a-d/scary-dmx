//
//  OscillatorController.h
//  Scary DMX
//
//  Created by Jason DiPrinzio on 11/15/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "oscillator_effect.h"

@interface OscillatorController : NSObject
{
    IBOutlet NSButton *control;
    IBOutlet NSSlider *high;
    IBOutlet NSSlider *low;
    IBOutlet NSSlider *speed;
    IBOutlet NSLevelIndicator *speedLevel;
    IBOutlet NSTextField *channelField;
    IBOutlet NSStepper *channelStepper;
    oscillator_data_t odata;
}
- (IBAction)toggleOscillator:(id)sender;
- (IBAction)updateHighThreshold:(id)sender;
- (IBAction)updateLowThreshold:(id)sender;
- (IBAction)updateSpeed:(id)sender;
- (IBAction)updateChannel:(id)sender;

@end
