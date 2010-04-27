//
//  ShowTableViewDS.m
//  Scary DMX
//
//  Created by Jason DiPrinzio on 10/10/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "ShowTableViewDS.h"
#import <QuickTime/QuickTime.h>
//#import "QTKit/QTMovie.h"

@implementation ShowTableViewDS

NSString *formatDuration(long duration)
{
    short left = 0;
    short hours = duration / 3600;
    left = duration % 3600;
    short mins = left / 60;
    short seconds = left % 60;
    NSString *ret = [NSString stringWithFormat:@"%02d:%02d:%02d", hours, mins, seconds];
    return ret;
}

-(int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if(showData){
        return numberOfRows;
    } else {
        return 0;
    }
}

-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    if(showData){
        NSString *col = [aTableColumn identifier];
        if(![col compare:@"0"]){
            return @"";
        }
        if(![col compare:@"1"]){
            id ret = nil;
            @try{
                ret = [durations objectAtIndex:rowIndex];
            } @catch (NSException *e) {
                NSLog(@"ShowTableViewDS::objectValueForTableColumn: %@, because %@", [e name], [e reason]);
            }
            return ret;
        }
        if(![col compare:@"2"]){
            if(showDataIndex[rowIndex]->cue->aData){
                return [NSString stringWithCString: showDataIndex[rowIndex]->cue->aData->movieFile];
            }else {
                return @"No sound effect";
            }
        }
        if(![col compare:@"3"]){
            return @"";
        }
        return @"";
    } else {
        return @"";        
    }
}

-(void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{

}

-(void)setShow:(dmx_show_t *) newShow
{
    showData = newShow;
    numberOfRows = newShow->cueCount;
    if(showDataIndex){
        free(showDataIndex);
    }
    showDataIndex = malloc(numberOfRows * (sizeof (cue_node_t *)));
    if(!showDataIndex){
        NSLog(@"Memory allocation failure.  Cannot allocate memory for show listing.");
    }
    cue_node_t *aCue = showData->currentCue;
    //
    short refId = 0;
    Movie *movie = 0;
    long duration = 0;
    //
    [durations dealloc];
    durations = [[NSMutableArray alloc] init];
    //
    int i =0;
    for(i=0; i< numberOfRows; i++){
        //map pointer array to each cue pointer;
        showDataIndex[i] = aCue;
        //get the movie duration for each movie in the list
        analyzer_data_t *aData = aCue->cue->aData;
        TimeValue v = 0;
        TimeScale s = 0;
        if(aData){
            const unsigned char *fileName = (unsigned char *)aCue->cue->aData->movieFile;
            if(open_movie_file(fileName, &movie, &refId)){
                continue;
            }
            if(movie){
                v = GetMovieDuration(*movie);
                s = GetMovieTimeScale(*movie);
                CloseMovieFile(refId);
                DisposeMovie(*movie);
                duration = v / s;
            } else{
                printf("Could not open movie to get duration: %s\n", fileName);
            }  
        }
        NSString *length = formatDuration(duration);
        if(length)
            [durations addObject:length];        
        //move to the next node in the list
        aCue = aCue->nextCue;
    }
    if(movie){
        free(movie);
    }
}

NSString *getHeaderNameForColumn(int index)
{
    switch(index){
        case 0:
            return @"Id";
        case 1:
            return @"Duration";
        case 2:
            return @"Audio";
        default:
            return @"Header";
    }
}

#define ED_COLS 4
#define COL_IS_EDITABLE(id) (( ED_COLS & (1 << id)) ? true : false)

-(void)setColumnHeaders:(NSArray *)columns
{
    NSTableColumn *col;
    int count = [columns count];
    int i=0;
    for( i=0; i< count; i++){
        col = [columns objectAtIndex:i];
        [[col headerCell] setStringValue: getHeaderNameForColumn(i)];
        [col setIdentifier:[NSString stringWithFormat:@"%d", i]];
        [col setEditable: COL_IS_EDITABLE(i) ]; 
        [col sizeToFit]; 
        if(2==i)
            [col setWidth:600];
    }
}

- (void) dealloc 
{
    if(showDataIndex){
        free(showDataIndex);
    }
    [super dealloc];
}

- (void) init
{
    showDataIndex = 0;
    [super init];
}

@end
