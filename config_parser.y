%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "show_handler.h"
#include "config_parser.h"
#include "config_parser.tab.h"

/* DECS */
static dmx_show_t *resultShow;

#define MAX_STEPS 128
static chaser_step_t *step_buffer[MAX_STEPS];
static uint8_t stepcount = 0;

int parse_show_file(const char *filename, dmx_show_t **show)
{
    FILE *showFile = fopen(filename, "r");
    if(!showFile) return -1;

    FILE *yyin;
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
    uint32_t time_val;// time based values
    double       dval;// threshold type values
    char        *text;
    struct {
        unsigned int count;
        unsigned int channels[512];
    } array;

    // complex data
    struct _effects_handle_t      *effect;
    struct _fader_data_t          *fader;
    struct _onoff_effect_t        *onoff;
    struct _flicker_data_t        *flicker;
    struct _oscillator_data_t     *oscillator;
    struct _chaser_step_t         *chaser_step;
    struct _chaser_t              *chaser;
}

/* types */
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
%type <val>             chase_value
%type <val>             repeat_value
%type <time_val>    predelay_value
%type <time_val>    postdelay_value
%type <text>              ERROR

/* effects */
%type <effect>       effect
%type <fader>        fader_effect
%type <onoff>        onoff_effect
%type <flicker>      flicker_effect
%type <oscillator>   oscillator_effect
%type <chaser_step>  chaser_step
%type <chaser_step>  chaser_steps
%type <chaser>       chaser_effect

/* token types */
%token <val>              VALUE
%token <time_val>       LONGVAL
%token <channel>        CHANNEL
%token <array>     CHANNEL_LIST
%token <dval>       FLOAT_VALUE
%token <text>         FILE_SPEC

%token ANALYZER BANDS CHAN CHASER CHASE_VALUE CUE DASH ERROR
%token FADER FILENAME FLICKER FREQ FROM TO
%token HIGH LBRACE LOW LPAREN ON_OFF OFFTIME OFFVALUE ONTIME ONVALUE
%token OSCILLATOR PRE_DELAY POST_DELAY REPEAT RBRACE RPAREN SEMICOLON SPEED STEP
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

    set_step_duration_for_current_cue(resultShow, 0);
    add_cue(resultShow);
}
|
CUE LPAREN VALUE RPAREN LBRACE settings RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Cue def with duration time: %d\n", $3);
#endif

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
analyzer_setting
|
timer_setting
|
flicker_effect
{
    set_flicker_for_current_cue(resultShow, $1);
}
|
oscillator_effect
{
    set_oscillator_data_for_current_cue(resultShow, $1);
}
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


/*** Handlers ***/
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

timer_setting:
TIMER LBRACE ontime_value offtime_value effect RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("Timer setting ontime: %d, offtime: %d, \n", $3, $4);
#endif
    timer_data_t* timer = NEW_TIMER(timer);
    timer->on_time = $3;
    timer->off_time = $4;
    timer->effect = $5;
    timer->timer_handle = 0;
    set_timer_data_for_current_cue(resultShow, timer);
}
;
/*** ***/

/*** EFFECTS ***/
effects:
effect
|
effects | effect
;

effect:
fader_effect
{
    effects_handle_t *effect = NEW_EFFECTS_HANDLE(effect);
    effect->type = effect_type_fader;
    effect->effect.fader = $1;
    yylval.effect = effect;
    $$ = yylval.effect;
}
|
onoff_effect
{
    effects_handle_t *effect = NEW_EFFECTS_HANDLE(effect);
    effect->type = effect_type_onoff;
    effect->effect.onoff = $1;
    yylval.effect = effect;
    $$ = yylval.effect;
}
|
flicker_effect
{
    effects_handle_t *effect = NEW_EFFECTS_HANDLE(effect);
    effect->type = effect_type_flicker;
    effect->effect.flicker = $1;
    yylval.effect = effect;
    $$ = yylval.effect;
}
|
oscillator_effect
{
    effects_handle_t *effect = NEW_EFFECTS_HANDLE(effect);
    effect->type = effect_type_oscillator;
    effect->effect.oscillator = $1;
    yylval.effect = effect;
    $$ = yylval.effect;
}
|
chaser_effect
{
    effects_handle_t *effect = NEW_EFFECTS_HANDLE(effect);
    effect->type = effect_type_chaser;
    effect->effect.chaser = $1;
    yylval.effect = effect;
    $$ = yylval.effect;
}
;

fader_effect:
FADER LBRACE channel_list from_value to_value speed_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("fader effect -- from: %d, to: %d, speed: %d\n", $4, $5, $6);
    log_debug(" %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug("%d \n", $3.channels[i]);
    }
#endif

    fader_data_t *fader = NEW_FADER_DATA(fader);
    fader->channels = channel_list_from_data($3.count, $3.channels);
    fader->from_value = $4;
    fader->to_value = $5;
    fader->speed = $6;

    yylval.fader = fader;
    $$ = yylval.fader;
}
;

onoff_effect:
ON_OFF LBRACE channel_list on_value off_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("on/off effect -- onvalue: %d, offvalue: %d\n", $4, $5);
    log_debug(" %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug("%d \n", $3.channels[i]);
    }
#endif
    onoff_effect_t *onoff = NEW_ONOFF_EFFECT(onoff);
    onoff->channels = channel_list_from_data($3.count, $3.channels);
    onoff->on_value = $4;
    onoff->off_value = $5;

    yylval.onoff = onoff;
    $$ = yylval.onoff;
}
;

oscillator_effect:
OSCILLATOR LBRACE channel_list low_value high_value speed_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("oscillator effect: low-- %d, high-- %d, speed-- %d\n", $4, $5, $6 );
    log_debug(" %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug(" %d\n", $3.channels[i]);
    }
    log_debug("\n");
#endif
    oscillator_data_t* oscillator = NEW_OSCILLATOR_DATA_T(oscillator);
    oscillator->channels = channel_list_from_data($3.count, $3.channels);
    oscillator->lowThreshold = $4;
    oscillator->highThreshold = $5;
    oscillator->speed = $6;
    yylval.oscillator = oscillator;
    $$ = yylval.oscillator;
}
;

flicker_effect:
FLICKER LBRACE channel_list RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("flicker effect: %d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug(" %d \n", $3.channels[i]);
    }
    log_debug("\n");
#endif
    flicker_data_t *flicker = NEW_FLICKER_DATA(flicker);
    flicker->channels = channel_list_from_data($3.count, $3.channels);
    yylval.flicker = flicker;
    $$ = yylval.flicker;
}
;

chaser_steps:
chaser_step
|
chaser_steps chaser_step
;

chaser_step:
STEP LBRACE channel_list chase_value repeat_value speed_value predelay_value postdelay_value RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("chaser step: chase value-- %d, repeat--%d, speed--%d, pre--%d, post--%d\n",
        $4, $5, $6, $7, $8);
    log_debug("%d channel(s)\n", $3.count);
    unsigned int i=0;
    for(i=0; i<$3.count; i++){
        log_debug(" %d \n", $3.channels[i]);
    }
#endif
    if(stepcount < MAX_STEPS) {
        chaser_step_t *step = NEW_CHASER_STEP(step);
        step->channels = channel_list_from_data($3.count, $3.channels);
        step->value = $4;
        step->repeat = $5;
        step->speed = $6;
        step->pre_delay = $7;
        step->post_delay = $8;
        step_buffer[stepcount++] = step;

        yylval.chaser_step = step;
        $$ = yylval.chaser_step;
    } else {
        log_warn("chaser step count exceeded... ignoring remaining steps for chaser\n");
    }
}
;

chaser_effect:
CHASER LBRACE chaser_steps RBRACE
{
#ifdef _TRACE_PARSER
    log_debug("chaser effect: %d steps\n", stepcount);
#endif
    chaser_t *chaser = NEW_CHASER(chaser);
    for(int i=0; i< stepcount; i++) {
        add_chaser_step(chaser, step_buffer[i]);
    }
    /* flush buffer */
    memset(step_buffer, 0, MAX_STEPS);
    yylval.chaser = chaser;
    $$ = yylval.chaser;
}
;

/**** ****/

/*** TYPES ***/
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

chase_value: CHASE_VALUE value
{
    $$ = $2;
}

repeat_value: REPEAT value
{
    $$ = $2;
}

predelay_value: PRE_DELAY time_value
{
    $$ = $2;
}

postdelay_value: POST_DELAY time_value
{
    $$ = $2;
}

value:          VALUE SEMICOLON
{
    $$ = $1;
}
;

time_value:
LONGVAL SEMICOLON
{
    $$ = $1;
}
|
VALUE SEMICOLON
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

