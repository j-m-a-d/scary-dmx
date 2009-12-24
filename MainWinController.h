/* MainWinController */

#import <Cocoa/Cocoa.h>

#ifndef SHOW_HANDLER_H
#include "show_handler.h"
#endif 

@interface MainWinController : NSObject
{
    IBOutlet NSButton *dmxButton;
    IBOutlet NSButton *showButton;
    IBOutlet NSProgressIndicator *progress;
    IBOutlet NSTableView *showList;
    IBOutlet NSTextField *statusText;
    IBOutlet NSObject *soundController;
    IBOutlet NSMenuItem *loopShow;
    IBOutlet NSTableView *showTable;
    NSString *showFile;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app;

/* Application actions */
- (IBAction)openShowFile:(id)sender;
- (IBAction)toggleDMX:(id)sender;
- (IBAction)toggleShow:(id)sender;
- (IBAction)updateChannel:(id)sender;
- (IBAction)setShowLoop:(id)sender;
- (void)startShow:(id)sender;
- (void)stopShow:(id)sender;
- (void)setPlayingCueTableRow;
- (void)nextCue:(id)sender;
- (void)rewindShow:(id)sender;

@end
