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

dmx_show_t *resultShow;

extern void yyerror(const char *str)
{
    log_error("error: %s\n",str);
}

extern int yywrap()
{
    return 1;
}

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
    unsigned int val;
    double  dval;
    char    *text;
    struct {
        unsigned int count;
        unsigned int channels[512];
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
%type <val>         on_value
%type <val>         off_value

%token <val>        VALUE
%token <val>        CHANNEL
%token <chan_list>  CHANNEL_LIST
%token <dval>       FLOAT_VALUE
%token <text>       FILE_SPEC
%token <text>       ERROR

%token CUE CHAN FLICKER OSCILLATOR ANALYZER 
%token TIMER SPEED LOW HIGH FILENAME TYPE FREQ THRESHOLD BANDS
%token THRESHOLD_VALUE DMX_VALUE ONTIME OFFTIME ONVALUE OFFVALUE
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON DASH

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
;

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
    int i=0;
    for(i=0; i<$3.count; i++){
        log_debug(" %d \n", $3.channels[i]);
    }
    log_debug("\n");
#endif
    channel_list_t chs = channel_list_from_data($3.count, $3.channels);
    set_flicker_channel_for_current_cue(resultShow, chs);
}
|
FLICKER LBRACE CHAN value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("flicker setting: %d\n", $4);
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
    log_debug("analyzer setting: %s \n", $3);
    log_debug("channel value: %d channel(s)\n", $4.count);
    int i=0;
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
|
ANALYZER LBRACE file_spec chan threshold threshold_value 
bands freq analyzer_type RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("analyzer setting: %s\n", $3);
    log_debug("channel value: %d\n", $4);
    log_debug("freq value: %d\n", $8);
    log_debug("threshold: %5.3f\n", $5);
    log_debug("threshold value: %d\n", $6);
    log_debug("bands: %d\n", $7);
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
    log_debug("Oscillator setting: low-- %d, high-- %d, speed-- %d\n", $4, $5, $6 );
    log_debug(" %d channel(s)\n", $3.count);
    int i=0;
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
|
OSCILLATOR LBRACE chan low_value high_value speed_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Oscillator setting: ch-- %d, low-- %d, high-- %d, speed-- %d\n", $3, $4, $5, $6 );
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
TIMER LBRACE chan ontime_value offtime_value on_value off_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Timer setting channel-- %d, ontime-- %d, offtime-- %d, onvalue-- %d, offvalue-- %d\n", $3, $4, $5, $6, $7);
#endif
    timed_effect_data_t* timer = NEW_TIMED_EFFECT(timer);
    timer->channels = new_channel_list(1);
    timer->channels->channels[1] = 0;
    timer->channels->channels[0] = $3;
    timer->on_time = $4;
    timer->off_time = $5;
    timer->on_value = $6;
    timer->off_value = $7;
    timer->timer_handle = 0;
    set_timer_data_for_current_cue(resultShow, timer);
}
|
TIMER LBRACE channel_list ontime_value offtime_value on_value off_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Timer setting ontime-- %d, offtime-- %d, on-value-- %d, off-value-- %d\n", $4, $5, $6, $7);
    log_debug(" %d channel(s)\n", $3.count);
    int i=0;
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

on_value: ONVALUE value
{
    $$ = $2;
}

off_value: OFFVALUE value
{
    $$ = $2;
}

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
    flog_debug(stderr, "%s not recognized.\n", $1);
    free($1);
}
;

%%

