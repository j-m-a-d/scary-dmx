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
/*
-(void) monitorFrequencies:(QTAudioFrequencyLevels*)levels
{
    if(!levels) return;
    register int i;
    for(i=0; i<_numberOfBands; i++){ 
        [levelIndicators[i] setFloatValue:((float)levels->level[i])*100.0];
        [levelIndicators[i+_numberOfBands] setFloatValue:((float)levels->level[i+_numberOfBands])*100.0];   
    }
}

void freqCallback(void* objRef, QTAudioFrequencyLevels* levels)
{
    id myself = (id)objRef;    
    if([myself respondsToSelector:@selector(monitorFrequencies:)]){
        [myself monitorFrequencies:levels];
    }        
}

-(void) startMonitor:(int)numberOfBands
{
    registerSelfAsFreqListener((void*)self, &freqCallback);
    _numberOfBands = numberOfBands;
}

-(void) stopMonitor
{
    register int i;
    for(i=0; i< _numberOfBands; i++){
        [levelIndicators[i] setFloatValue:0.0];
        [levelIndicators[i+_numberOfBands] setFloatValue:0.0];
    }
}
*/
@end
