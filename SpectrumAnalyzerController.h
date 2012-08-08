/*
 *  SpectrumDisplayController.h
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 11/19/11.
 *  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
 */

#import <Cocoa/Cocoa.h>
#import "SpectrumAnalyzerView.h"

@interface SpectrumAnalyzerController : NSObject {
    IBOutlet NSPanel *window;
    IBOutlet SpectrumAnalyzerView *analyzer;
}

- (void)windowWillClose:(NSNotification *)notification;
- (void)windowDidResize:(NSNotification *)notification;

@end
