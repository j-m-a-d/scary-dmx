//
//  SpectrumAnalyzerView.m
//  Scary DMX
//
//  Created by Jason Diprinzio on 11/8/11.
//  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
//

#import "SpectrumAnalyzerView.h"
#import <OpenGL/OpenGL.h>
#import <Quicktime/Quicktime.h>
#import <sound_analyzer.h>
#include <string.h>

@implementation SpectrumAnalyzerView

static float levels[10] = {0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0};

static void draw_analyzer_graph (int num_bars, float levels[])
{
    
    float light_q = 1.0f;
    
    glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(0.0f, 0.0f, 1.0f);
	
	GLfloat ambientLight[] = {0.3f, 0.3f, 0.3f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	
	GLfloat lightColor[] = {0.7f, 0.7f, 0.7f, 1.0f};
	GLfloat lightPos[] = {2 * light_q, light_q, 10 * light_q, 1.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	glTranslatef(-20.0f, -6.5f, -20.0f);
	
    float bar_width = (40.0f / (float)num_bars) - 0.1f ;
    
    register int i=0;
    
    for(i=0; i<num_bars; i++){ 
        glBegin(GL_QUADS);
        glColor3f(0.0f, 0.5f, 0.8f);
        glNormal3f(0.0, -1.0f, 0.0f);
        
        float bar_height = (float)(11.0f * levels[i]);
        
        glVertex3f(0.0f, 0.0f, 1.0f );
        glVertex3f(0.0f, bar_height, 1.0f );
        glVertex3f(bar_width  , bar_height, 1.0f );
        glVertex3f(bar_width  , 0.0f, 1.0f ); 
        
        glEnd();	
        
        glTranslatef(bar_width+0.1f, 0.0f, 0.0f);
    }    
    glFlush();
}

inline static void set_rect(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,(float)width / (float)height, 1.0f, 200.0f);
    glClearColor(0, 0, 0, 0);
}

-(void) drawRect: (NSRect) bounds
{
    NSSize size = [self bounds].size;
    set_rect(size.width,size.height);
    draw_analyzer_graph(10, levels);
}

-(void) prepareOpenGL
{
    [super prepareOpenGL];
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_MULTISAMPLE);
    [self startAnalyzer];
}

-(void) monitorFrequencies:(QTAudioFrequencyLevels*)freq_levels
{
    if(!freq_levels) return;
    //NSSize size = [self bounds].size;
    //int num_bands = freq_levels->numFrequencyBands;
    
    memcpy(levels, freq_levels->level, sizeof(freq_levels->level));
    [self drawRect:[self bounds]];
    //draw_analyzer_graph(num_bands, levels);
}

void freqCallback(void* objRef, QTAudioFrequencyLevels* freq_levels)
{
    id myself = (id)objRef; 
    int num_bands = freq_levels->numFrequencyBands;
    memcpy(levels, freq_levels->level, sizeof(freq_levels->level));
    [[myself openGLContext] makeCurrentContext];
    draw_analyzer_graph(num_bands, freq_levels->level);
    /*
    if([myself respondsToSelector:@selector(monitorFrequencies:)]){
        [myself monitorFrequencies:levels];
    }
     */
}

-(void) startAnalyzer
{
    registerSelfAsFreqListener((void*)self, &freqCallback);
}
/*
+ (NSOpenGLPixelFormat*)defaultPixelFormat
{
    return [[NSOpenGLPixelFormat alloc] init];
    NSOpenGLPixelFormatAttribute attr[] =
    {
        NSOpenGLPFANoRecovery,
		NSOpenGLPFAWindow,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADoubleBuffer, 
		NSOpenGLPFAColorSize, 24,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 24,
		NSOpenGLPFAStencilSize, 8,
		NSOpenGLPFAAccumSize, 0,
		0
    };
    
	return [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
}

- (id)initWithFrame:(NSRect)frameRect
        pixelFormat:(NSOpenGLPixelFormat*)format
{
    self = [super initWithFrame:frameRect];
    if (self != nil) {
        _pixelFormat   = [format retain];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_surfaceNeedsUpdate:)
                                                     name:NSViewGlobalFrameDidChangeNotification
                                                   object:self];
    }
    return self;
}

- (void)drawRect:(NSRect)rect
{
    [_openGLContext makeCurrentContext];
	glViewport(0, 0, rect.size.width, rect.size.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glColor3f(1.0f, 0.85f, 0.35f);
    //glTranslatef(0.0f, 0.0f, 0.0f);
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(  0.0,  0.6, 0.0);
        glVertex3f( -0.2, -0.3, 0.0);
        glVertex3f(  0.2, -0.3 ,0.0);
    }
    glEnd();
    glFlush();    
    [_openGLContext flushBuffer];
}

- (void)dealloc{
    [[NSNotificationCenter defaultCenter] 
     removeObserver:self
     name:NSViewGlobalFrameDidChangeNotification
     object:self];
    
    [self clearGLContext];
    [_pixelFormat release];
    [super dealloc];
}

- (void)update
{
    if ([_openGLContext view] == self) {
        [_openGLContext update];
    }
}

- (void) _surfaceNeedsUpdate:(NSNotification*)notification
{
    [self update];
}

-(void) setOpenGLContext:(NSOpenGLContext *)context
{
    _openGLContext = [context retain];
}

-(NSOpenGLContext*) openGLContext
{
    return _openGLContext;
}

-(void) clearGLContext
{
}

-(void) prepareOpenGL
{
    NSOpenGLPixelFormat *format = [SpectrumAnalyzerView defaultPixelFormat];
   	_openGLContext = [[NSOpenGLContext alloc] initWithFormat: format shareContext: nil];
	[_openGLContext makeCurrentContext];
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
    
}

-(void) setPixelFormat:(NSOpenGLPixelFormat *)pixelFormat
{
    _pixelFormat = [pixelFormat retain];
}

-(NSOpenGLPixelFormat*) pixelFormat
{
    return _pixelFormat;
}
*/
@end

