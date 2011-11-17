//
//  DMXOutputDisplayController.h
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/14/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpectrumAnalyzerView.h"


@interface DMXOutputDisplayController : NSObject {
    IBOutlet NSPanel *window;
    IBOutlet SpectrumAnalyzerView *analyzer;
    NSTimer *timer;
    float buffer[16];
}

@end
