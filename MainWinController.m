#import "MainWinController.h"
#import "SoundController.h"
#import "ShowTableViewDS.h"

@implementation MainWinController

#define LAST_OPENED_SHOW  @"lastOpenedShow"
#define LAST_OPENED_SHOW_DIR @"lastOpenedShowDir"

- (void)terminate
{
    stop_show();
    free_all_show();
    destroy_dmx();
    ExitMovies();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app
{
    [self terminate];
    return NSTerminateNow;
}

- (IBAction)openShowFile:(id)sender
{
    int result = 0;
    NSArray *fileTypes = [NSArray arrayWithObjects:@"shw", nil];
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];

    [oPanel setAllowsMultipleSelection:NO];
    [oPanel setTitle:@"Choose File"];
    [oPanel setMessage:@"Choose show file to open."];
    [oPanel setDelegate:self];
    // Go to the last opend directory if there was one.
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *lastShowDir = (NSString*)[defaults objectForKey:LAST_OPENED_SHOW_DIR];
    NSString *path = NSHomeDirectory();
    if(nil != lastShowDir){
        path = lastShowDir;
    }
    // Launch the dialog.    
    result = [oPanel runModalForDirectory:path file:nil types:fileTypes];
    if (NSOKButton == result) {
        [self stopShow: self];
        NSString *newFile = [oPanel filename];
        [defaults setObject:[newFile stringByDeletingLastPathComponent] forKey:LAST_OPENED_SHOW_DIR];
        dmx_show_t *newShow = 0;
        result = load_show_from_file([newFile cStringUsingEncoding:NSUTF8StringEncoding], &newShow );
        if(!result){
            [statusText setStringValue:[newFile lastPathComponent]];
            NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
            [defaults setObject:newFile forKey:LAST_OPENED_SHOW];
            ShowTableViewDS *ds = (ShowTableViewDS *)[showTable dataSource];
            [ds setShow:newShow];
            [showTable reloadData];
        }else{
            NSAlert *alert = [[NSAlert alloc] init];
            [alert addButtonWithTitle:@"OK"];
            [alert setMessageText:@"Error loading show file."];
            [alert setAlertStyle:NSWarningAlertStyle];
            [alert runModal];
			[alert release];
        }
    }
}

- (IBAction)toggleDMX:(id)sender
{
    if( NSOnState == [dmxButton state] ){
        start_dmx();
    } else {
        stop_dmx();
    }
}

static unsigned int _current_cue_index = 0;

- (void)startShow:(id)sender
{
    [progress startAnimation:sender];
    [showList setEnabled:FALSE];
    //TODO: highlight first cue in list
    NSIndexSet *row = [NSIndexSet indexSetWithIndex:_current_cue_index];
    [showTable selectRowIndexes:row byExtendingSelection:NO];
    id sc = (SoundController*)soundController;
    [sc startMonitor:7];
}

- (void)stopShow:(id)sender
{
    if([loopShow state] && nil == sender){
        start_show();
    } else {
        [showButton setState: NSOffState];
        [progress stopAnimation:sender];
        [showList setEnabled:TRUE];
        id sc = (SoundController*)soundController;
        [sc stopMonitor];
    }
}

- (void)setPlayingCueTableRow
{
    ShowTableViewDS *ds = [showTable dataSource];
    if(_current_cue_index >= [ds numberOfRowsInTableView:showTable] -1)
        _current_cue_index = -1;
    NSIndexSet *row = [NSIndexSet indexSetWithIndex:++_current_cue_index];
    [showTable selectRowIndexes:row byExtendingSelection:NO];
}

- (void)doNextStep 
{
    [self setPlayingCueTableRow];
}

- (void)nextCue:(id)sender
{
    if(!skip_cue()){
        [self setPlayingCueTableRow];    
    }    
}

- (void)rewindShow:(id)sender
{
    rewind_show();
    _current_cue_index = -1;
    [self setPlayingCueTableRow];
}

void show_has_ended(void *objRef)
{
    id mySelf = (id)objRef;
    if([mySelf respondsToSelector:@selector(stopShow:)]){
        [mySelf stopShow: nil];
    }
}

void show_next_step(void *objRef, cue_node_t *cueData)
{
    id mySelf = (id)objRef;
    if([mySelf respondsToSelector:@selector(doNextStep:)]){
        [mySelf doNextStep];
    }
}

- (IBAction)toggleShow:(id)sender
{
    if( NSOnState == [showButton state] ){
        start_show();
        [self startShow: sender];
    } else {
        stop_show();
        [self stopShow: sender];
    }
}

- (IBAction)updateChannel:(id)sender
{
    NSSlider* curSlider = (NSSlider*)sender;
    update_channel([curSlider tag], [curSlider intValue]);
}

- (IBAction)setShowLoop:(id)sender
{
    [loopShow setState: ![loopShow state]];
}

- (void)awakeFromNib
{
    int result =0;
    result = init_dmx();
    if(DMX_INIT_OK != result){
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        if(DMX_INIT_NO_DEVICES == result){
            [alert setMessageText:@"Could not find DMX device."];
        } else {
            [alert setMessageText:@"Error initializing DMX device."];
        }
        [alert setAlertStyle:NSWarningAlertStyle];
        [alert runModal];
		[alert release];
        /* TODO create a way to run the app even if no device was configured. */
    }
    EnterMovies();
    // Setup show table
    ShowTableViewDS *ds = [[ShowTableViewDS alloc] init];
    [showTable setDataSource:ds];
    [ds setColumnHeaders:[showTable tableColumns]];
    // Load a show if we can find the last show opened.
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *lastShow = (NSString*)[defaults objectForKey:LAST_OPENED_SHOW];
    dmx_show_t *newShow;
    if(nil != lastShow){
        int result = load_show_from_file([lastShow cStringUsingEncoding:NSUTF8StringEncoding], &newShow);
        if(!result){
            [statusText setStringValue:[lastShow lastPathComponent]];
            [ds setShow:newShow];
            [showTable reloadData];
        } else {
            [defaults removeObjectForKey:LAST_OPENED_SHOW];
        }
    }
    register_show_ended((void *)self, &show_has_ended);
    register_show_next_step((void *)self, &show_next_step);
}

@end
