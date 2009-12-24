//
//  ShowTableViewDS.h
//  Scary DMX
//
//  Created by Jason DiPrinzio on 10/10/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#ifndef SHOW_HANDLER_H
#include "show_handler.h"
#endif


@interface ShowTableViewDS :NSObject {
    int numberOfRows;
    dmx_show_t *showData;
    cue_node_t **showDataIndex;
    NSMutableArray *durations;
}

-(int)numberOfRowsInTableView:(NSTableView *)aTableView;
-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
-(void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;

-(void)setShow:(dmx_show_t *)newShow;
-(void)setColumnHeaders:(NSArray *)columns;
@end
