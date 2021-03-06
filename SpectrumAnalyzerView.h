/*
 *  SpectrumAnalyzerView.h
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 11/8/11.
 *  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
 */

#import <Cocoa/Cocoa.h>

#define MAX_ANALYZER_LEVELS 51
#define INITIAL_LEVEL_COUNT 41

@interface SpectrumAnalyzerView : NSOpenGLView {
    float _levels[MAX_ANALYZER_LEVELS];
    unsigned int _levelCount;
    
    NSTimer *timer;
}

-(void)start;
-(void)stop;
-(void)update:(unsigned int)count :(float *)newLevels;

/*    
@private
    NSOpenGLContext*   _openGLContext;
    NSOpenGLPixelFormat* _pixelFormat;
}

+ (NSOpenGLPixelFormat*)defaultPixelFormat;
- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format;
- (void)setOpenGLContext:(NSOpenGLContext*)context;
- (NSOpenGLContext*)openGLContext;
- (void)clearGLContext;
- (void)prepareOpenGL;
- (void)update;
- (void)setPixelFormat:(NSOpenGLPixelFormat*)pixelFormat;
- (NSOpenGLPixelFormat*)pixelFormat;
*/

@end
