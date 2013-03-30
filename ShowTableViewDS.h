/*
 *  ShowTableViewDS.h
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 10/10/09.
 *  Copyright 2009 Inspirotech Inc. All rights reserved.
 */

#import <Cocoa/Cocoa.h>
#ifndef SHOW_HANDLER_H
#include "show_handler.h"
#endif


@interface ShowTableViewDS :NSObject <NSTableViewDelegate, NSTableViewDataSource> {
    unsigned int _numberOfRows;
    dmx_show_t *_showData;
    cue_node_t **_showDataIndex;
    NSMutableArray *durations;
}

-(void)setShow:(dmx_show_t *)newShow;
-(void)setColumnHeaders:(NSArray *)columns;

/* NSTableViewDataSource */
-(int)numberOfRowsInTableView:(NSTableView *)aTableView;
-(id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
-(void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex;
-(BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation;
-(NSArray *)tableView:(NSTableView *)aTableView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedRowsWithIndexes:(NSIndexSet *)indexSet;
-(void)tableView:(NSTableView *)aTableView sortDescriptorsDidChange:(NSArray *)oldDescriptors;
-(NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation;
-(BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard;

/* NSTableViewDelegate */
-(void)tableView:(NSTableView *)aTableView willDisplayCell:(id)aCell forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex;


@end
