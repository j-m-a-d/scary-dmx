/*
 *  SpectrumAnalyzerController.m
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 11/19/11.
 *  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
 */

#import "SpectrumAnalyzerController.h"


@implementation SpectrumAnalyzerController

- (IBAction)show:(id)sender
{
    [panel makeKeyAndOrderFront:nil];
    [analyzer start];
}

- (void)windowWillClose:(NSNotification *)notification
{
    [analyzer stop];
}

-(void)windowDidResize:(NSNotification *)notification
{
    NSRect rect = [[panel contentView] bounds];
    rect.origin.x = rect.origin.x + 20;
    rect.origin.y = rect.origin.y + 20;
    rect.size.width = rect.size.width - 40;
    rect.size.height = rect.size.height - 40;
    
    [analyzer setFrame:rect];
    [analyzer drawRect:rect];
}

@end
