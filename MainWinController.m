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
    free_all_show();
    destroy_dmx();
    ExitMovies();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)app
{
    [self terminate];
    return NSTerminateNow;
}

static unsigned short _current_cue_index = 0;

- (NSURL*)chooseShowFile:(NSString*)fromDir:(NSArray*)fileTypes:(NSString*)action
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

- (IBAction)openShowFile:(id)sender
{
    /* Go to the last opend directory if there was one. */
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *lastShowDir = (NSString*)[defaults objectForKey:LAST_OPENED_SHOW_DIR];
    NSArray *fileTypes = [NSArray arrayWithObjects:@"shw", nil];
    NSURL *path = [self chooseShowFile:lastShowDir:fileTypes: @"Open"];
            
    if (nil != path) {
        [self stopShow: self];
        NSString *newFile = [path path];
        [defaults setObject:[newFile stringByDeletingLastPathComponent] forKey:LAST_OPENED_SHOW_DIR];
        dmx_show_t *newShow = 0;
        if(!load_show_from_file([newFile cStringUsingEncoding:NSUTF8StringEncoding], &newShow )){
            [statusText setStringValue:[newFile lastPathComponent]];
            [defaults setObject:newFile forKey:LAST_OPENED_SHOW];
            _current_cue_index = 0;
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
    if(_current_cue_index >= [ds numberOfRowsInTableView:showTable]) _current_cue_index=0;
    NSIndexSet *row = [NSIndexSet indexSetWithIndex:_current_cue_index];
    [showTable selectRowIndexes:row byExtendingSelection:NO];
    [pool release];
}

- (void)doNextStep:(int)cue_id
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
    update_channel([curSlider tag], (dmx_value_t)[curSlider intValue]);
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
    [defaults release];
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
