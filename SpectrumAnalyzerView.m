/*
 *  SpectrumAnalyzerView.m
 *  Scary DMX
 *
 *  Created by Jason Diprinzio on 11/8/11.
 *  Copyright 2011 Inspirotech Consulting, Inc. All rights reserved.
 */

#import "SpectrumAnalyzerView.h"
#import <string.h>
#import <OpenGL/OpenGL.h>
#import <OPenGL/glu.h>


@implementation SpectrumAnalyzerView

float LEVELS[MAX_ANALYZER_LEVELS] = {0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f,0.9f,0.8f,0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.1f,0.0f,
    0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,0.9f,1.0f,0.9f,0.8f,0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.1f,0.0f};


static void draw_analyzer_graph (unsigned int num_levels, float levels[])
{
    float spacing = 0.5f;
    float max_w = 40.f;
    float max_h = 12.0f;
    
    glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(0.0f, 0.0f, 1.0f);
	
	GLfloat ambientLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
	
	//GLfloat lightColor[] = {0.9f, 0.9f, 0.9f, 1.0f};
	//GLfloat lightPos[] = {-20.0f, 0.0f, 1.0f, 1.0f};
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	//glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	glTranslatef(-20.0f, -6.5f, -20.0f);
	
    float bar_width = (max_w / (float)num_levels) - spacing ;
    
    register unsigned int i=0;
    float *cur = levels;
    
    for(i=0; i<num_levels; i++, cur++){ 
        glBegin(GL_QUADS);
        
        glColor3f(0.0f, 0.5f, 0.8f);
        glNormal3f(0.0f, -1.0f, 0.0f);
        
        float bar_height = (float)(max_h * (*cur) );
        
        glVertex3f(0.0f, 0.0f, 1.0f );
        glVertex3f(0.0f, bar_height, 1.0f );
        
        glColor3f(0.0f, 0.3f, 0.8f);
        glNormal3f(0.0f, -1.0f, 0.0f);
        
        glVertex3f(bar_width  , bar_height, 1.0f );
        glVertex3f(bar_width  , 0.0f, 1.0f ); 
        
        glEnd();	
        
        glTranslatef(bar_width+spacing, 0.0f, 0.0f);
    }
    
    glFlush();
}

static inline void display_setrect(float width, float height)
{
    glViewport(0, 0, (int)width, (int)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0,(float)width / (float)height, 1.0f, 200.0f);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
}

static void display_prepare()
{
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_SMOOTH);
}

-(void) drawRect: (NSRect)bounds
{
    NSSize size = bounds.size;
    [[self openGLContext] makeCurrentContext];
    display_setrect(size.width, size.height);
    draw_analyzer_graph(_levelCount, _levels);
}

-(void) prepareOpenGL
{
    [super prepareOpenGL];
   
    _levelCount = MAX_ANALYZER_LEVELS;
    memcpy(_levels, LEVELS, MAX_ANALYZER_LEVELS * sizeof(float));

    [[self openGLContext] makeCurrentContext];
    display_prepare();
}

-(void)update:(unsigned int)count :(float*)newLevels
{
    _levelCount = count;
    /* TODO check against max levels */
    memcpy(_levels, newLevels, _levelCount * sizeof(float));
}

-(void)reduce
{
    float *cur = _levels;
    register unsigned int i= 0;
    for(i=0; i<MAX_ANALYZER_LEVELS; i++){
        float nv = *cur - .005f;
        *cur = (nv > 0.0f ? nv : 0.0f);
        cur++;
    }
}

-(void)drawIt
{
    [[self openGLContext] makeCurrentContext];
    draw_analyzer_graph(_levelCount, _levels);
    [self reduce];
}

-(void) start
{
    timer = [NSTimer scheduledTimerWithTimeInterval:
             .01
             target:self 
             selector:@selector(drawIt) 
             userInfo:nil 
             repeats:YES];
    [timer retain];
}

-(void) stop
{
    [timer invalidate];
    [timer release];
}   
    
@end

