//
//  SpectrumAnalyzerView.h
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/8/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface SpectrumAnalyzerView : NSOpenGLView {
}
-(void)startAnalyzer;
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