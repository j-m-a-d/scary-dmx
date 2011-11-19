//
//  AnalyzerDisplayHandler.h
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/8/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpectrumAnalyzerController.h"

@interface FrequencyAnalyzerDisplayController : SpectrumAnalyzerController {

}

-(void)updateAnalyzer:(int)count: (float*)levels;

@end
