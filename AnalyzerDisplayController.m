//
//  AnalyzerDisplayHandler.m
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/8/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import "AnalyzerDisplayController.h"


@implementation AnalyzerDisplayController

- (void)awakeFromNib
{
    [analyzer prepareOpenGL];
    [analyzer startAnalyzer];
}
     
@end
