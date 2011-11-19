//
//  DMXOutputDisplayController.h
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/14/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpectrumAnalyzerController.h"


@interface DMXOutputDisplayController : SpectrumAnalyzerController {
    NSTimer *timer;
    float buffer[16];
}

@end
