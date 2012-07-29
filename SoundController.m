#import "SoundController.h"

@implementation SoundController

int _numberOfBands;

-(void) awakeFromNib
{
    [levelViewLeft setFrameRotation:90.0];
    [levelViewRight setFrameRotation:90.0];
    register int i;
    for(i=0; i<7; i++){
        id li = [levelViewLeft viewWithTag:i];
        [li setEnabled:FALSE];
        [li setMaxValue:100.0];
        [li setMinValue:0.0];
        levelIndicators[i] = li;
        li = [levelViewRight viewWithTag:i];
        [li setEnabled:FALSE];
        [li setMaxValue:100.0];
        [li setMinValue:0.0];
        levelIndicators[i+7] = li;
    }
}

@end
