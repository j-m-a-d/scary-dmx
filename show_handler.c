/*
 *  show_handler.c
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 9/21/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "show_handler.h"
#include "timed_effect.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static dmx_show_t *_live_show = 0;

static pthread_t _show_pt = 0;

enum show_state {
    SHOW_STATE_STOPPED    =1 << 0,
    SHOW_STATE_STOPPING   =1 << 1,
    SHOW_STATE_STARTING   =1 << 2,
    SHOW_STATE_RUNNING    =1 << 3,
    SHOW_STATE_SKIPPING   =1 << 4
};

static enum show_state _state;

static pthread_mutex_t _control_mutex = PTHREAD_MUTEX_INITIALIZER;

static void(*_call_show_end)(void*) = 0;
static void *_show_end_obj = 0;

static void(*_call_show_next_step)(void*, cue_node_t *) = 0;
static void *_show_next_step_obj = 0;

#define SHOWING() \
    (SHOW_STATE_RUNNING & _state)

#define SHOW_STOPPED() \
    (SHOW_STATE_STOPPED & _state)

#define SHOW_INTRANSIT() \
    ( 0x16 & _state )

/*
    Advance the current cue in the show.
 */
static int advance_cue(dmx_show_t *show)
{
    show->currentCue = show->currentCue->nextCue;
    return (NULL == show->currentCue || NULL == show->currentCue->cue ||
            show->currentCue->cue->empty);
}

/*
    Release all the memory allocated
    for this show structure.
 */
int free_show(dmx_show_t *show)
{
    if(!show){
        return 0;
    }
    if(!(show->currentCue)){
        return 0;
    }
    while(show->currentCue->nextCue){
        show->currentCue = show->currentCue->nextCue;
    }
    while(show->currentCue){
        cue_node_t *tmp = show->currentCue;
        show->currentCue = tmp->previousCue;
        free(tmp->cue->channelValues);
        tmp->cue->channelValues = 0;
        FREE_ANALYZER_DATA (tmp->cue->aData);
        FREE_OSCILLATOR_DATA (tmp->cue->oData);
        if(tmp->cue){
            if(tmp->cue->channelValues)

            
            FREE_TIMED_EFFECTS(tmp->cue->timer);
            
            DELETE_CHANNEL_LIST (tmp->cue->flickerChannels);
            
            free(tmp->cue);
            tmp->cue = 0;
        }
        memset(tmp, 0, sizeof(cue_node_t));//<<<----
        free(tmp);
        tmp = 0;
    }
    memset(show, 0, sizeof(dmx_show_t));
    free(show);
    show = 0;
    return 0;
}

int free_all_show()
{
    FREE_SHOW(_live_show);
    return 0;
}

/*
    Allocate and create a cue node.
 */
int create_cue_node(cue_node_t **cueNode)
{
    *cueNode = malloc(sizeof(cue_node_t));
    if(NULL == cueNode){
        fprintf( stderr, "Cannot create cue node.  Supplied value is zero.\n");
        return 1;
    }
    
    if(NULL == *cueNode){
        fprintf(stderr, "Could not allocate memory for cue.\n");
        *cueNode = 0;
        return 1;
    }
    /* Initialze each member. */
    (*cueNode)->cue = (cue_t*)malloc(sizeof(cue_t));
    memset( (*cueNode)->cue, 0, sizeof(cue_t));
    (*cueNode)->cue->empty = 1;
    (*cueNode)->cue->flickerChannels=0;
    (*cueNode)->cue->stepDuration=0;
    (*cueNode)->cue->aData=0;
    (*cueNode)->cue->oData=0;
    (*cueNode)->cue->timer=0;
    (*cueNode)->previousCue = 0;
    (*cueNode)->nextCue = 0;
    /* Allocate the channel values */
    (*cueNode)->cue->channelValues = calloc(sizeof(dmx_value_t), DMX_CHANNELS);
    if( NULL == (*cueNode)->cue->channelValues){
        fprintf(stderr, "Could not allocate memory for channel settings.\n");
        free( (*cueNode)->cue );
        (*cueNode)->cue = 0;
        //(*cueNode)->cue->channelValues = 0;
        return 1;
    }
    memset((*cueNode)->cue->channelValues, 0, DMX_CHANNELS);
    return 0;
}

/* 
    Allocate memory and initialize a show
    with its first cue.
 */
int init_show(dmx_show_t **show)
{
    *show = (dmx_show_t*)malloc(sizeof(dmx_show_t));
    if(NULL == show){
        fprintf(stderr, "Could not allocate memory for show.\n");
        *show = 0;
        return 1;
    }
    (*show)->showName = "Default Show";
    (*show)->cueCount = 0;
    (*show)->currentCue = 0;
    if(create_cue_node(&((*show)->currentCue))){
        free( *show );
        *show = 0;
        return 1;
    }
    (*show)->currentCue->cue_id = 0;
    
    return 0;
}

/*
    Print a list of cue channels from a show to a file.
 */
static void printCueChannels(unsigned char *channels, FILE *showFile)
{
    unsigned char *cv = channels;
    register int i;
    for(i=0; i< DMX_CHANNELS+1; i++){
        if(*cv){
            fprintf(showFile, "\tch%d:%d;\n", i, (int)(*cv));
        }
        cv++;
    }
}

/*
    Print a list of channels for any effect that supports channes lists.
 */
static void printChannelList(channel_list_t channels, FILE *showFile)
{
    dmx_channel_t *tmp = channels->channels;
    fprintf(showFile, "\t\t ch:%d", *tmp++);
    while(*tmp){
        fprintf(showFile, ",%d", *tmp);
        tmp++;
    }
    fprintf(showFile, ";\n");
}

/*
    Print an analyzer setting to a show file.
 */
static void printAnalyzer(analyzer_data_t *data, FILE *showFile)
{
    fprintf(showFile, "\tanalyzer {\n");
    fprintf(showFile, "\t\t file:%s;\n", data->movieFile);
    printChannelList(data->dmxChannelList, showFile);
    fprintf(showFile, "\t\t threshold:%6.3f;\n", data->threshold);
    fprintf(showFile, "\t\t threshold_value:%d;\n", data->dmxValue);
    fprintf(showFile, "\t\t bands:%lu;\n", data->numberOfBandLevels);
    fprintf(showFile, "\t\t freq:%d;\n", data->frequency);
    fprintf(showFile, "\t\t type:%d;\n", data->flags);
    fprintf(showFile, "\t}\n");
}

/*
    Print an oscillator setting to a show file.
 */
static void printOscillatorData(oscillator_data_t *data, FILE *showFile)
{
    fprintf(showFile, "\toscillator {\n");
    printChannelList(data->dmxChannels, showFile);
    fprintf(showFile, "\t\t low:%d;\n", data->lowThreshold);
    fprintf(showFile, "\t\t high:%d;\n", data->highThreshold);
    fprintf(showFile, "\t\t speed:%d;\n", data->speed);
    fprintf(showFile, "\t}\n");
}

/*
    Print a timer setting to a show file.
 */
static void printTimerData(timed_effect_data_t *data, FILE *showFile)
{
    fprintf(showFile, "\ttimer {\n");
    fprintf(showFile, "\t\t ch:%d;\n", data->channel);
    fprintf(showFile, "\t\t dmx-value:%d;\n", data->value);
    fprintf(showFile, "\t\t ontime:%ld;\n", data->on_time);
    fprintf(showFile, "\t\t offtime:%ld;\n", data->off_time);
    fprintf(showFile, "\t}\n");
}

/*
    Print a flicker value to a show file.
 */
static void printFlickerChannels(channel_list_t dmxChannels, FILE *showFile)
{
    fprintf(showFile, "\tflicker {\n");
    printChannelList(dmxChannels, showFile);
    fprintf(showFile, "\t}\n");
}

/*
    Print a show to a file.
 */
void printShow(dmx_show_t *show, FILE *showFile)
{
    cue_node_t *node = show->currentCue;
    cue_t *cue;        
    while(node){
        cue = node->cue;
        fprintf(showFile, "cue ");
        if(cue->stepDuration) fprintf(showFile, "(%d)", cue->stepDuration);
        fprintf(showFile, "{\n");        
        printCueChannels(cue->channelValues, showFile);  
        if(cue->flickerChannels) printFlickerChannels(cue->flickerChannels, showFile);
        if(cue->aData) printAnalyzer(cue->aData, showFile);  
        if(cue->oData) printOscillatorData(cue->oData, showFile);
        if(cue->timer) printTimerData(cue->timer, showFile);
        fprintf(showFile, "}\n");        
        node = node->nextCue;             
    }    
}

/*
    Add a cue to this show at the 
    end of the cue list.
 */
int add_cue(dmx_show_t* show)
{
    cue_node_t *lastCue = show->currentCue;
    while(lastCue->nextCue){
        lastCue = lastCue->nextCue;
    }
    cue_node_t *newCue;
    int id = lastCue->cue_id +1;
    int i = create_cue_node(&newCue);
    if(i){
        /* free newCue and it's children */
        return i;
    }
    newCue->cue_id = id;
    lastCue->nextCue = newCue;
    newCue->previousCue = lastCue;
    newCue->nextCue = 0; // <===-----
    show->currentCue = newCue;
    show->cueCount++;
    return 0;
}

inline void set_channel_value_for_current_cue(dmx_show_t *show, int ch, int val)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->channelValues[ch]=val;
}

inline void set_step_duration_for_current_cue(dmx_show_t *show, int duration)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->stepDuration = duration;
}

inline void set_flicker_channel_for_current_cue(dmx_show_t *show, channel_list_t ch)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->flickerChannels = ch;
}

inline void set_oscillator_data_for_current_cue(dmx_show_t *show, oscillator_data_t *oData)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->oData = oData;
}

inline void set_analyzer_data_for_current_cue(dmx_show_t *show, analyzer_data_t *aData)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->aData = aData;
}

void set_timer_data_for_current_cue(dmx_show_t *show, timed_effect_data_t *data)
{
    if(NULL == show->currentCue->cue->timer){
        show->currentCue->cue->timer = data;
        show->currentCue->cue->timer->nextTimer = NULL;
    } else {
        show->currentCue->cue->empty = 0;
        timed_effect_data_t *tmp = show->currentCue->cue->timer;
        show->currentCue->cue->timer = data;
        show->currentCue->cue->timer->nextTimer = tmp;
    }
}

static void go_to_next_step();

/*
 Thread function to invoke the next step in the show.
 */
static void *next_step()
{
    if(!SHOWING()){
        goto die_now;
    }
    
	int result = 0;
    stop_oscillating();
    stop_flicker();
    if(_live_show->currentCue->previousCue)
        stop_timed_effects(_live_show->currentCue->previousCue->cue->timer);
        
    cue_node_t *cueNode = _live_show->currentCue;
    cue_t *cue = cueNode->cue;
     
    if(cue->aData && !cue->stepDuration){
        result = start_analyze(cue->aData, &go_to_next_step);
		if(result){
			goto die_now;
		}
    }
    
    bulk_update(cue->channelValues);
     
    if(cue->flickerChannels){
        start_flicker(cue->flickerChannels);        
    }
    
    if(cue->oData){
        start_oscillating(cue->oData);
    }
    
    if(cue->timer){
        /* do each timer */
        timed_effect_data_t *tdata = cue->timer;
        while(NULL != tdata){
            timed_effect_handle *timer = 0;
            if(!tdata->timer_handle){
                create_timed_effect_handle(&timer);
                tdata->timer_handle = timer;
            }
            cue_timed_effect(tdata);
            tdata = tdata->nextTimer;
        }
        /* now start them all at once */
        start_timed_effects();
    }
die_now:
	pthread_exit(NULL);

}

static void _stop_show()
{
    /*
     The show thread is only 'alive' during
     the setup of a cue.  It exits immediately
     but in case a call to stop_show comes immediately
     after a call to start_show we'll wait for the show
     thread to exit first.
     */
    if(0 != _show_pt){
        pthread_join(_show_pt, NULL);
        _show_pt = 0;
    }
    stop_analyze();
    stop_oscillating();
    stop_flicker();
    stop_timed_effects(NULL);
}

/*
 Stop the show sequence.
 */
void stop_show()
{
    if(SHOW_STOPPED())
        return;
    
    pthread_mutex_lock(&_control_mutex);
    _state = SHOW_STATE_STOPPING;
    
    _stop_show();
    
    _state = SHOW_STATE_STOPPED;
    pthread_mutex_unlock(&_control_mutex);
}

/*
 Callback from movie thread to know when to move to the next scene.
 */
static void go_to_next_step()
{
    int showOver = 0;
    if(SHOWING()){
        /* Cue up the next scene. */
        showOver = advance_cue(_live_show); 
        if(!showOver){        
            pthread_create(&_show_pt, NULL, &next_step, NULL);
            if(_call_show_next_step){
                /* notify listeners */
                _call_show_next_step(_show_next_step_obj, _live_show->currentCue);
            }
        } else {
            stop_show();
            _rewind_show(_live_show);
            if(_call_show_end){
                /* notify listeners */
                _call_show_end(_show_end_obj);
            }
        }
    }
}

/*
 Load a show file.
 */
int parse_show_file(const char *filename, dmx_show_t **show);
int load_show_from_file(const char *show_file, dmx_show_t **out_show)
{
    stop_show();
    dmx_show_t *newShow;
    int result = parse_show_file(show_file, &newShow);
    if(!result && NULL != newShow){
        if(_live_show != NULL) {
            FREE_SHOW(_live_show);
        }
        _live_show = newShow;
        *out_show = _live_show;
    }
    return result;
}

/*
 Set a pre-built show to be played.
 */
int setShow(dmx_show_t *show)
{
    if(SHOW_STOPPED())
        return 1;
    
    free_all_show();
    _live_show = show;
    return 0;
}

/*
 Only called from start_show()
 */
static void _start_show()
{
    pthread_create(&_show_pt, NULL, &next_step, NULL);
}

/*
 Start the show sequence.
 */
int start_show()
{
    if(NULL == _live_show) return DMX_SHOW_NOT_LOADED;
    if(SHOW_STATE_RUNNING==_state) return 0;
    
    pthread_mutex_lock(&_control_mutex);
    _state = SHOW_STATE_STARTING;
    
    _start_show();
    
    _state = SHOW_STATE_RUNNING;
    pthread_mutex_unlock(&_control_mutex);
    
    return DMX_SHOW_OK;
}

/*
 Reset the cue pointer in the show
 to point to the first cue.
 */
void _rewind_show(dmx_show_t *show)
{
    if(show == 0) return;
    while((show->currentCue->previousCue)){
        show->currentCue = show->currentCue->previousCue;
    }
}

/*
 Rewind the live show.  Callable from external
 */
void rewind_show()
{
    pthread_mutex_lock(&_control_mutex);
    enum show_state lastState = _state;
    _state = SHOW_STATE_SKIPPING;


    if(SHOW_STATE_RUNNING == lastState){
        _stop_show();
        _rewind_show(_live_show);
        _start_show();
    } else {
        _rewind_show(_live_show);
    }

    _state = lastState;
    pthread_mutex_unlock(&_control_mutex);
}

/*
 Skip to the next cue
 */
int skip_cue()
{
    if(SHOW_INTRANSIT())
        return -1;

    pthread_mutex_lock(&_control_mutex);
    enum show_state last_state = _state;

    _state = SHOW_STATE_SKIPPING;

    if(SHOW_STATE_RUNNING & last_state) {
        skip_movie();
    } else if(_live_show && _live_show->currentCue->nextCue){
        _live_show->currentCue = _live_show->currentCue->nextCue;
        _call_show_next_step(_show_next_step_obj, _live_show->currentCue);
    }

    _state = last_state;
    
    pthread_mutex_unlock(&_control_mutex);

    return 0;
}

/*
 Register show events
 */
void register_show_ended(void *callRef, void(*showEnded)(void*))
{
    _call_show_end = showEnded;
    _show_end_obj = callRef;
 }
 
void register_show_next_step(void *callRef, void(*show_next_step)(void*, cue_node_t*))
{
    _call_show_next_step = show_next_step;
    _show_next_step_obj = callRef;
}
