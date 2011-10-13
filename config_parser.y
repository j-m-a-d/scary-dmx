%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "show_handler.h"
 
dmx_show_t *resultShow;
    
void yyerror(const char *str)
{
    fprintf(stderr,"error: %s\n",str);
}

int yywrap()
{
    return 1;
} 

int parse_show_file(const char *filename, dmx_show_t **show)
{
    FILE *showFile = fopen(filename, "r");
    if(!showFile){
        return -1;
    }
    //
    extern FILE *yyin;
    yyin = showFile;
    //
    int i = init_show(&resultShow); 
    if(i) return i;
    //
    yyrestart(showFile);
    i = yyparse();
    if(i) return i;
    //
    _rewind_show(resultShow);   
    fclose(showFile);
    //
    *show = resultShow;
    resultShow = 0;
    //
    return 0; 
}

#ifdef _EXT_PARSER
int main(int argc, char **argv)
{
    extern FILE *yyin;
    yyin = fopen(argv[1], "r");
    //
    int i = init_show(&resultShow);
    if(i){
        fprintf(stderr, "Failed to initialize show.");
    }
    i = yyparse();
    if(i){
        fprintf(stderr, "Parse error.");
        return i;
    }
    _rewind_show(resultShow);    
    //
    FILE *outFile = fopen("./outshow.shw", "w+");
    if(!outFile){
        printf("Could not open out file for show output.");
    }
    printShow(resultShow, outFile);
    fclose(outFile);
    //
    FREE_SHOW (resultShow);
    return i;
}
#endif

#ifdef _CLI_MAIN
#include <signal.h>
#include <unistd.h>
#include <QuickTime/QuickTime.h>
#include <Carbon/Carbon.h>

void sig_all(int sig)
{
    sigset_t mask_set;
    sigset_t old_set;
    
    signal(sig, sig_all);
    sigfillset(&mask_set);
    sigprocmask(SIG_SETMASK, &mask_set, &old_set);
    
	stop_show();
	free_all_show();
	destroy_dmx();
	ExitMovies();
	fprintf(stderr, "Shutdown finished.\n");
	exit(0);
}

int main(int argc, char **argv)
{
    (void)signal(SIGINT, &sig_all);
    (void)signal(SIGQUIT, &sig_all);
    (void)signal(SIGKILL, &sig_all);

    EnterMovies();
        
    dmx_show_t *newShow;
    char *showFile;
    showFile = argv[1];
    if(!showFile){
        fprintf(stderr, "Usage: %s showfile\n", argv[0]);
        return 1;
    }
    if(load_show_from_file(showFile, &newShow )){
        fprintf(stderr, "Show not found or invalid show.\n");
        return 2;
    }

    if(DMX_INIT_OK != init_dmx()){
        fprintf(stderr, "Failed to open DMX device.\n");
        return 3;
    }
    
    start_dmx();
    fprintf(stdout, "Starting show.\n");
    start_show();
    fprintf(stdout, "Running....");
    while(1){
        //usleep(100000);
        sleep(5);
    }
    
    return 0;
}
#endif

%} 

%union {
    int     val;
    double  dval;
    char    *text;
    struct {
        int count;
        int channels[512];
    } chan_list;
}

%type <val>         value
%type <dval>        float_value
%type <chan_list>   channel_list
%type <text>        file_spec
%type <val>         analyzer_type
%type <dval>        threshold
%type <val>         threshold_value
%type <val>         bands
%type <val>         freq
%type <val>         chan
%type <val>         low_value
%type <val>         high_value
%type <val>         speed_value
%type <val>         dmx_value
%type <val>         ontime_value
%type <val>         offtime_value

%token <val>        VALUE
%token <val>        CHANNEL
%token <chan_list>  CHANNEL_LIST
%token <dval>       FLOAT_VALUE
%token <text>       FILE_SPEC
%token <text>       ERROR

%token CUE CHAN FLICKER OSCILLATOR ANALYZER 
%token TIMER SPEED LOW HIGH FILENAME TYPE FREQ THRESHOLD BANDS
%token THRESHOLD_VALUE DMX_VALUE ONTIME OFFTIME 
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON DASH

%token <text> UNKNOWN

%%

show: 
cues
{
#ifdef _TRACE_PARSER
    printf("Show def.\n");
#endif
}
;

cues: 
cue
|
cues cue
;

cue:
CUE LBRACE RBRACE
{
#ifdef _TRACE_PARSER
    printf("Empty Cue def.\n");
#endif
}
|
CUE LBRACE settings RBRACE   
{
#ifdef _TRACE_PARSER
    printf("Cue def.\n");
#endif
//
    set_step_duration_for_current_cue(resultShow, 0);
    add_cue(resultShow);
}
|
CUE LPAREN VALUE RPAREN LBRACE settings RBRACE
{
#ifdef _TRACE_PARSER
    printf("Cue def with duration time: %d\n", $3);
#endif
//
    set_step_duration_for_current_cue(resultShow, $3);
    add_cue(resultShow);
}
;

settings: 
setting
|
settings setting
;

setting:
channel_setting 
|
flicker_setting
|
analyzer_setting
|
oscillator_setting
|
timer_setting
;

channel_setting: 
CHANNEL  value 
{
#ifdef _TRACE_PARSER
    printf("set channel %d : %d\n", $1, $2);
#endif
    set_channel_value_for_current_cue(resultShow, $1, $2);
}
;

flicker_setting:
FLICKER LBRACE channel_list RBRACE
{
#ifdef _TRACE_PARSER
    printf("flicker setting: ");
    printf(" %d channel(s)", $3.count);
    int i=0;
    for(i=0; i<$3.count; i++){
        printf(" %d, ", $3.channels[i]);
    }
    printf("\n");
#endif
    channel_list_t chs = channel_list_from_data($3.count, $3.channels);
    set_flicker_channel_for_current_cue(resultShow, chs);
}
|
FLICKER LBRACE CHAN value RBRACE
{
#ifdef _TRACE_PARSER
    printf("flicker setting: %d\n", $4);
#endif
    channel_list_t chs = new_channel_list(1);
    chs->channels[1] = 0;
    chs->channels[0] = $4;
    set_flicker_channel_for_current_cue(resultShow, chs);
}
;

analyzer_setting:
ANALYZER LBRACE file_spec channel_list threshold threshold_value 
bands freq analyzer_type RBRACE
{
#ifdef _TRACE_PARSER
    printf("analyzer setting: %s ,", $3);
    printf("channel value: ");
    printf(" %d channel(s)", $4.count);
    int i=0;
    for(i=0; i<$4.count; i++){
        printf(" %d, ", $4.channels[i]);
    }
    printf("freq value: %d ,", $8);
    printf("threshold: %5.3f ,", $5);
    printf("threshold value: %d ,", $6);
    printf("bands: %d\n", $7);
#endif
    analyzer_data_t *aData = NEW_ANALYZER_DATA_T(aData);
    aData->movieFile = $3;
    aData->dmxChannelList = channel_list_from_data($4.count, $4.channels);
    aData->threshold = $5;
    aData->dmxValue = $6;
    aData->numberOfBandLevels = $7;
    aData->frequency = $8;
    aData->flags = $9;
    set_analyzer_data_for_current_cue(resultShow, aData);
}
|
ANALYZER LBRACE file_spec chan threshold threshold_value 
bands freq analyzer_type RBRACE
{
#ifdef _TRACE_PARSER
    printf("analyzer setting: %s ,", $3);
    printf("channel value: %d ,", $4);
    printf("freq value: %d ,", $8);
    printf("threshold: %5.3f ,", $5);
    printf("threshold value: %d ,", $6);
    printf("bands: %d\n", $7);
#endif
    analyzer_data_t *aData = NEW_ANALYZER_DATA_T(aData);
    aData->movieFile = $3;
    aData->dmxChannelList = new_channel_list(1);
    aData->dmxChannelList->channels[1] = 0;
    aData->dmxChannelList->channels[0] = $4;
    aData->threshold = $5;
    aData->dmxValue = $6;
    aData->numberOfBandLevels = $7;
    aData->frequency = $8;
    aData->flags = $9;
    set_analyzer_data_for_current_cue(resultShow, aData);
}           
;

oscillator_setting:
OSCILLATOR LBRACE channel_list low_value high_value speed_value RBRACE
{
#ifdef _TRACE_PARSER
    printf("Oscillator setting: low-- %d, high-- %d, speed-- %d", $4, $5, $6 );
    printf(" %d channel(s)", $3.count);
    int i=0;
    for(i=0; i<$3.count; i++){
        printf(" %d, ", $3.channels[i]);
    }
    printf("\n");
#endif
    oscillator_data_t* oData = NEW_OSCILLATOR_DATA_T(oData);
    oData->dmxChannels = channel_list_from_data($3.count, $3.channels);
    oData->lowThreshold = $4;
    oData->highThreshold = $5;
    oData->speed = $6;
    set_oscillator_data_for_current_cue(resultShow, oData);
}
|
OSCILLATOR LBRACE chan low_value high_value speed_value RBRACE
{
#ifdef _TRACE_PARSER
    printf("Oscillator setting: ch-- %d, low-- %d, high-- %d, speed-- %d\n", $3, $4, $5, $6 );
#endif
    oscillator_data_t* oData = NEW_OSCILLATOR_DATA_T(oData);
    oData->dmxChannels = new_channel_list(1);
    oData->dmxChannels->channels[1] = 0;
    oData->dmxChannels->channels[0] = $3;
    oData->lowThreshold = $4;
    oData->highThreshold = $5;
    oData->speed = $6;
    set_oscillator_data_for_current_cue(resultShow, oData);
}
;

timer_setting:
TIMER LBRACE chan dmx_value ontime_value offtime_value RBRACE
{
#ifdef _TRACE_PARSER
    printf("Timer setting channel-- %d, on-- %d, off-- %d\n",$3, $5, $6);
#endif
    timed_effect_data_t* timer = malloc(sizeof(timed_effect_data_t));
    memset(timer, 0, sizeof(timed_effect_data_t));
    timer->channel = $3;
    timer->value = $4;
    timer->on_time = $5;
    timer->off_time = $6;
    timer->timer_handle = 0;
    set_timer_data_for_current_cue(resultShow, timer);
}
;

analyzer_type:
TYPE value
{
    $$ = $2;
}
;

freq:
FREQ value
{
    $$ = $2;
}
;

chan:           CHAN value
{
    $$ = $2;
}
;

threshold:      THRESHOLD float_value
{
    $$ = $2;
}
;

threshold_value: THRESHOLD_VALUE value
{
    $$ = $2;
}
;

bands:          BANDS value
{
    $$ = $2;
}
;

dmx_value:      DMX_VALUE value
{
    $$ = $2;
}
;

low_value:      LOW value
{
    $$ = $2;
}
;

high_value:     HIGH value
{
    $$ = $2;
}
;

speed_value:    SPEED value
{
    $$ = $2;
}
;

ontime_value:   ONTIME value
{
    $$ = $2;
}
;

offtime_value:  OFFTIME value
{
    $$ = $2;
}
;

value:          VALUE SEMICOLON
{
    $$ = $1;
}
;

float_value:    FLOAT_VALUE SEMICOLON
{   
    $$ = $1;
}
;

file_spec:      FILENAME FILE_SPEC SEMICOLON
{
    $$ = $2;
}
;

channel_list:   CHAN CHANNEL_LIST SEMICOLON
{
    $$ = $2;
}
;

err : ERROR
{
    fprintf(stderr, "%s not recognized.\n", $1);
    free($1);
    //free_show(resultShow);    
}
;

%%

