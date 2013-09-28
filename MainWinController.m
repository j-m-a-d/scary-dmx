#import "MainWinController.h"
#import "show_handler.h"
#import "SoundController.h"
#import "ShowTableViewDS.h"

@implementation MainWinController

#define LAST_OPENED_SHOW  @"lastOpenedShow"
#define LAST_OPENED_SHOW_DIR @"lastOpenedShowDir"

- (void)terminate
{
    stop_show();
    free_loaded_show();
    destroy_dmx();
    ExitMovies();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app
{
    [self terminate];
    return NSTerminateNow;
}

static unsigned int _current_cue_index = 0;

- (NSURL*)chooseShowFile:(NSString*)fromDir :(NSArray*)fileTypes :(NSString*)action
{
    NSOpenPanel *oPanel = [NSOpenPanel openPanel];
    
    [oPanel setAllowsMultipleSelection:NO];
    [oPanel setTitle:@"Choose File"];
    [oPanel setMessage:[NSString stringWithFormat:@"Choose show file to %@.", action]];
    [oPanel setDelegate:self];
    
    NSString *path = NSHomeDirectory();
    if(nil != fromDir){
        path = fromDir;
    }
    /* Launch the dialog. */
    [oPanel setAllowedFileTypes:fileTypes];
    [oPanel setDirectoryURL:[NSURL URLWithString:path]];
    if(NSOKButton == [oPanel runModal]) {
        return [[oPanel URL] filePathURL];
    }
    return nil;
}

- (int)loadShowFile:(id)sender :(NSString*)show
{
    dmx_show_t *newShow = 0;
    [self stopShow:sender];
    int result = load_show_from_file([show cStringUsingEncoding:NSUTF8StringEncoding], &newShow);
    if(0 == result){
        [statusText setStringValue:[show lastPathComponent]];
        _current_cue_index = 0;
        [ds setShow:newShow];
        [showTable reloadData];
    }
    return result;
}

-(void) loadError
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Error loading show file."];
    [alert setAlertStyle:NSWarningAlertStyle];
    [alert runModal];
    [alert release];
}

- (IBAction)reloadShowFile:(id) sender
{
    if([self loadShowFile:sender: showFile]) {
        [self loadError];
    }
}

- (IBAction)openShowFile:(id)sender
{
    /* Go to the last opend directory if there was one. */
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *lastShowDir = (NSString*)[defaults objectForKey:LAST_OPENED_SHOW_DIR];
    NSArray *fileTypes = [NSArray arrayWithObjects:@"shw", nil];
    [showFile release];
    showFile = [[self chooseShowFile:lastShowDir:fileTypes: @"Open"] path];
    [showFile retain];
    if (nil != showFile) {
        [self stopShow: self];
        [defaults setObject:[showFile stringByDeletingLastPathComponent] forKey:LAST_OPENED_SHOW_DIR];
        if([self loadShowFile:sender: showFile]){
            [self loadError];            
        }else {
            [defaults setObject:showFile forKey:LAST_OPENED_SHOW];
        }
    }
}

- (IBAction)toggleDMX:(id)sender
{
    if( NSOnState == [dmxButton state] ){
        start_dmx();
        reset_channel_values_for_current_cue();
    } else {
        stop_dmx();
    }
}

- (void)startShow:(id)sender
{
    [progress startAnimation:sender];
    [showList setEnabled:FALSE];
    [self setPlayingCueTableRow];
}

- (void)stopShow:(id)sender
{
    if([loopShow state] && nil == sender){
        _current_cue_index = 0;
        [self setPlayingCueTableRow];
        start_show();
    } else {
        [showButton setState: NSOffState];
        [progress stopAnimation:sender];
        [showList setEnabled:TRUE];
    }
}

- (void)setPlayingCueTableRow
{
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    if(_current_cue_index >= (unsigned int)[ds numberOfRowsInTableView:showTable]) _current_cue_index=0;
    NSIndexSet *row = [NSIndexSet indexSetWithIndex:_current_cue_index];
    [showTable selectRowIndexes:row byExtendingSelection:NO];
    [pool release];
}

- (void)doNextStep:(unsigned int)cue_id
{
    _current_cue_index = cue_id;
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
    _current_cue_index = 0;
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
        [mySelf doNextStep: cueData->cue_id];
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
    update_channel((dmx_channel_t)[curSlider tag], (dmx_value_t)[curSlider intValue]);
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
    
    /* Setup show table */
    [ds setColumnHeaders:[showTable tableColumns]];
    
    /* Load a show if we can find the last show opened. */
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults retain];
    NSString *lastShow = (NSString*)[defaults objectForKey:LAST_OPENED_SHOW];
    if(nil != lastShow){
        if([self loadShowFile:self: lastShow]) {
            [defaults removeObjectForKey:LAST_OPENED_SHOW];
        }
    }
    [defaults release];
    showFile = lastShow;
    [showFile retain];
    register_show_ended((void *)self, &show_has_ended);
    register_show_next_step((void *)self, &show_next_step);

}

- (void)dealloc
{
    [ds release];
    [ds dealloc];
    [super dealloc];
}
@end
