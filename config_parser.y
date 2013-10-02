%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "show_handler.h"
#include "config_parser.h"
#include "config_parser.tab.h"


/* DECS */
extern void yyrestart(FILE*);
extern int yyparse();
extern int yylex();
extern int yyerror();

dmx_show_t *resultShow;

extern int parse_show_file(const char *filename, dmx_show_t **show)
{
    FILE *showFile = fopen(filename, "r");
    if(!showFile){
    return -1;
    }

    extern FILE *yyin;
    yyin = showFile;

    int i = init_show(&resultShow);
    if(i) return i;

    yyrestart(showFile);
    i = yyparse();
    if(i) return i;

    _rewind_show(resultShow);
    fclose(showFile);

    *show = resultShow;
    resultShow = 0;

    return 0;
}


%}

%union {
    uint8_t       val;// dmx values
    uint32_t  channel;// channel numbers
    uint64_t time_val;// time based values
    double       dval;// threshold type values
    char        *text;
    struct {
        unsigned int count;
        unsigned int channels[512];
    } array;
}

%type <val>               value
%type <time_val>     time_value
%type <dval>        float_value
%type <array>      channel_list
%type <text>          file_spec
%type <val>       analyzer_type
%type <dval>          threshold
%type <val>     threshold_value
%type <val>               bands
%type <val>                freq
%type <val>           low_value
%type <val>          high_value
%type <time_val>    speed_value
%type <time_val>   ontime_value
%type <time_val>  offtime_value
%type <val>            on_value
%type <val>           off_value
%type <val>          from_value
%type <val>            to_value
%type <text>              ERROR

%token <val>              VALUE
%token <time_val>       LONGVAL
%token <channel>        CHANNEL
%token <array>     CHANNEL_LIST
%token <dval>       FLOAT_VALUE
%token <text>         FILE_SPEC


%token ANALYZER BANDS CHAN CUE DASH ERROR
%token FADER FILENAME FLICKER FREQ FROM TO
%token HIGH LBRACE LOW LPAREN OFFTIME OFFVALUE ONTIME ONVALUE
%token OSCILLATOR RBRACE RPAREN SEMICOLON SPEED
%token THRESHOLD THRESHOLD_VALUE TIMER TYPE

%token <text> UNKNOWN

%%

show: 
cues
{
#ifdef _TRACE_PARSER
    log_debug("Show def.\n");
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
    log_debug("Empty Cue def.\n");
#endif
}
|
CUE LBRACE settings RBRACE   
{
#ifdef _TRACE_PARSER
    log_debug("Cue def.\n");
#endif
//
    set_step_duration_for_current_cue(resultShow, 0);
    add_cue(resultShow);
}
|
CUE LPAREN VALUE RPAREN LBRACE settings RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Cue def with duration time: %d\n", $3);
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
|
fader_setting
|
syntax_error
;

syntax_error:
ERROR
{
    log_error("%s\n", $1);
    free($1);
}

channel_setting: 
CHANNEL  value 
{
#ifdef _TRACE_PARSER
    log_debug("set channel %d : %d\n", $1, $2);
#endif
    set_channel_value_for_current_cue(resultShow, $1, $2);
}
;

flicker_setting:
FLICKER LBRACE channel_list RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("flicker setting: %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug(" %d \n", $3.channels[i]);
    }
    log_debug("\n");
#endif
    channel_list_t chs = channel_list_from_data($3.count, $3.channels);
    set_flicker_channel_for_current_cue(resultShow, chs);
}
;

analyzer_setting:
ANALYZER LBRACE file_spec channel_list threshold threshold_value 
bands freq analyzer_type RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("analyzer setting: %s \n", $3);
    log_debug("channel value: %d channel(s)\n", $4.count);
    unsigned int i=0;
    for(i=0; i<$4.count; i++){
        log_debug("%d \n", $4.channels[i]);
    }
    log_debug("freq value: %d \n", $8);
    log_debug("threshold: %5.3f \n", $5);
    log_debug("threshold value: %d \n", $6);
    log_debug("bands: %d\n", $7);
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
;

oscillator_setting:
OSCILLATOR LBRACE channel_list low_value high_value speed_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Oscillator setting: low-- %d, high-- %d, speed-- %lld\n", $4, $5, $6 );
    log_debug(" %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug(" %d\n", $3.channels[i]);
    }
    log_debug("\n");
#endif
    oscillator_data_t* oData = NEW_OSCILLATOR_DATA_T(oData);
    oData->dmxChannels = channel_list_from_data($3.count, $3.channels);
    oData->lowThreshold = $4;
    oData->highThreshold = $5;
    oData->speed = $6;
    set_oscillator_data_for_current_cue(resultShow, oData);
}
;

timer_setting:
TIMER LBRACE channel_list ontime_value offtime_value on_value off_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Timer setting ontime-- %lld, offtime-- %lld, on-value-- %d, off-value-- %d\n", $4, $5, $6, $7);
    log_debug(" %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
    log_debug(" %d\n", $3.channels[i]);
    }
    log_debug("\n");
#endif
    timed_effect_data_t* timer = NEW_TIMED_EFFECT(timer);
    timer->channels = channel_list_from_data($3.count, $3.channels);
    timer->on_time = $4;
    timer->off_time = $5;
    timer->on_value = $6;
    timer->off_value = $7;
    timer->timer_handle = 0;
    set_timer_data_for_current_cue(resultShow, timer);
}
;

fader_setting:
FADER LBRACE channel_list from_value to_value speed_value RBRACE
{
    log_debug("from: %d, to: %d, speed: %lld\n", $4, $5, $6);
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

from_value:     FROM value
{
    $$ = $2;
}

to_value:       TO value
{
    $$ = $2;
}

speed_value:    SPEED time_value
{
    $$ = $2;
}
;

ontime_value:   ONTIME time_value
{
    $$ = $2;
}
;

offtime_value:  OFFTIME time_value
{
    $$ = $2;
}
;

on_value: ONVALUE value
{
    $$ = $2;
}
;

off_value: OFFVALUE value
{
    $$ = $2;
}
;

value:          VALUE SEMICOLON
{
    $$ = $1;
}
;

time_value:  LONGVAL SEMICOLON
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

channel_list: 
CHAN CHANNEL_LIST SEMICOLON
{
    $$ = $2;
}
|
CHAN value
{
    yylval.array.count = 1;
    yylval.array.channels[0] = $2;
    $$ = yylval.array;
}
;

%%

