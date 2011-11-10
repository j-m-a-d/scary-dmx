/* SoundController */

#import <Cocoa/Cocoa.h>
#import <Quicktime/Quicktime.h>
#import <sound_analyzer.h>

@interface SoundController : NSObject
{
    IBOutlet NSView *levelViewLeft;
    IBOutlet NSView *levelViewRight;
    NSLevelIndicator* levelIndicators[14];
}
/*
-(void) startMonitor:(int)numberOfBands;
-(void) stopMonitor;
*/
@end
