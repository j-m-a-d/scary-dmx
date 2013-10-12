/*
 *  show_handler.c
 *  Scary DMX
 *
 *  Created by Jason DiPrinzio on 9/21/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "show_handler.h"
#include "config_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static dmx_show_t *_live_show = 0;

static pthread_t _show_pt = 0;
static pthread_mutex_t _control_mutex = PTHREAD_MUTEX_INITIALIZER;
static volatile enum op_state _state;

static void(*_call_show_end)(void*) = 0;
static void *_show_end_obj = 0;

static void(*_call_show_next_step)(void*, cue_node_t *) = 0;
static void *_show_next_step_obj = 0;

/*
 * Advance the current cue in the show.
 */
static int advance_cue(dmx_show_t *show)
{
    if(!show->currentCue) return 0;

    show->currentCue = show->currentCue->nextCue;
    return (NULL == show->currentCue || NULL == show->currentCue->cue ||
            show->currentCue->cue->empty);
}

#define FREE_CUE(cue) \
    free_cue(cue); \
    cue = 0;

static void free_cue(cue_t *cue)
{
    if(!cue) return;

    if(cue->channelValues)
        free(cue->channelValues);

    cue->channelValues = 0;

    FREE_ANALYZER_DATA (cue->aData);
    FREE_OSCILLATOR_DATA (cue->oData);
    FREE_TIMED_EFFECTS(cue->timer);
    FREE_FLICKER_DATA(cue->flickerChannels);

    memset(cue, 0, sizeof(cue_t));
    free(cue);
}

static void free_cue_node(cue_node_t *node)
{
    if(node) {
        free_cue(node->cue);
    }
    free(node);
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
    /* Skip to the end and work backwards. */
    while(show->currentCue->nextCue){
        show->currentCue = show->currentCue->nextCue;
    }

    while(show->currentCue){
        cue_node_t *tmp = show->currentCue;
        show->currentCue = tmp->previousCue;

        if(tmp->cue){
            FREE_CUE(tmp->cue);
        }

        memset(tmp, 0, sizeof(cue_node_t));
        free(tmp);
        tmp = 0;
    }

    memset(show, 0, sizeof(dmx_show_t));
    free(show);
    show = 0;
    return 0;
}

int free_loaded_show()
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
        log_error( "Could not allocate memory for cue.\n");
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
        log_error( "Could not allocate memory for channel settings.\n");
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
        log_error( "Could not allocate memory for show.\n");
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
        print_cue_channels(cue->channelValues, showFile);
        if(cue->flickerChannels) print_flicker_channels(cue->flickerChannels, showFile);
        if(cue->aData) print_analyzer(cue->aData, showFile);
        if(cue->oData) print_oscillator_data(cue->oData, showFile);
        if(cue->timer) print_timer_data(cue->timer, showFile);
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
    unsigned int id = lastCue->cue_id +1;
    int i = create_cue_node(&newCue);
    if(i){
        /* TODO free newCue and it's children */
        free_cue_node(newCue);
        return i;
    }
    newCue->cue_id = id;
    lastCue->nextCue = newCue;
    newCue->previousCue = lastCue;
    newCue->nextCue = 0;
    show->currentCue = newCue;
    show->cueCount++;
    return 0;
}

inline void set_channel_value_for_current_cue(dmx_show_t *show, dmx_channel_t ch, dmx_value_t val)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->channelValues[ch] = val;
}

inline void set_step_duration_for_current_cue(dmx_show_t *show, unsigned int duration)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->stepDuration = duration;
}

inline void set_flicker_for_current_cue(dmx_show_t *show, flicker_data_t *flicker_data)
{
    show->currentCue->cue->empty = 0;
    show->currentCue->cue->flickerChannels = flicker_data;
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

void set_timer_data_for_current_cue(dmx_show_t *show, timer_data_t *data)
{
    if(NULL == show->currentCue->cue->timer){
        show->currentCue->cue->timer = data;
        show->currentCue->cue->timer->nextTimer = 0;
    } else {
        show->currentCue->cue->empty = 0;
        timer_data_t *tmp = show->currentCue->cue->timer;
        show->currentCue->cue->timer = data;
        show->currentCue->cue->timer->nextTimer = tmp;
    }
}

void reset_channel_values_for_current_cue()
{
    if(_live_show) {
        if(_live_show->currentCue){
            if(_live_show->currentCue->cue) {
                bulk_update(_live_show->currentCue->cue->channelValues);
            }
        }
    }
}

static void go_to_next_step();

/*
 Thread function to invoke the next step in the show.
 */
static void *next_step()
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.showhandler");

    if(!RUNNING(_state)){
        log_debug( "Show is not in progress, exiting.\n");
        goto die_now;
    }

    cue_node_t *cueNode = _live_show->currentCue;
    cue_t *cue = cueNode->cue;

    if(cue->aData && !cue->stepDuration){
        int err = start_analyze(cue->aData, &go_to_next_step);
        if(err){
            log_error( "Cannot start sound analyzer: %d\n", err);
            goto die_now;
        }
    }

    reset_channel_values_for_current_cue();

    if(cue->flickerChannels){
        start_flicker(cue->flickerChannels);
    }

    if(cue->oData){
        start_oscillating(cue->oData);
    }

    timers_init();

    if(cue->timer){
        /* do each timer */
        timer_data_t *tdata = cue->timer;
        while(NULL != tdata){
            if(!tdata->timer_handle){
                create_timer_handle(&(tdata->timer_handle));
            }
            cue_timer(tdata);
            tdata = tdata->nextTimer;
        }
        /* now start them all at once */
        start_timers();
    }

die_now:
    EXIT_THREAD();
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
    if(_live_show)
        stop_timers(_live_show->currentCue->cue->timer);
}

/*
 Stop the show sequence.
 */
void stop_show()
{
    if(STOPPED(_state))
        return;

    pthread_mutex_lock(&_control_mutex);
    _state = OP_STATE_STOPPING;

    _stop_show();

    _state = OP_STATE_STOPPED;
    pthread_mutex_unlock(&_control_mutex);
}

/*
 Callback from movie thread to know when to move to the next scene.
 */
static void go_to_next_step()
{
    log_debug("Callback: moving to next cue from: %s.\n", _live_show->currentCue->cue->aData->movieFile);

    int showOver = 0;

    log_debug("Stopping effects.\n");

    stop_oscillating();
    log_debug("Stopped oscillator.\n");

    stop_flicker();
    log_debug("Stopped flicker.\n");

    if(_live_show->currentCue)
        stop_timers(_live_show->currentCue->cue->timer);
    log_debug("Stopped timed effects.\n");

    log_debug("showstate=%#x.\n", _state);

    if(RUNNING(_state)){
        /* Cue up the next scene. */
        showOver = advance_cue(_live_show);
        log_debug("Advanced to next cue.\n");
        if(!showOver){
            log_debug( "Calling next step.\n");
            spawn_joinable_pthread(&_show_pt, &next_step, NULL);
            log_debug( "Returned from next step.\n");
            if(_call_show_next_step){
                /* notify listeners */
                _call_show_next_step(_show_next_step_obj, _live_show->currentCue);
            }
        } else {
            log_debug( "Show is over.\n");
            stop_show();
            _rewind_show(_live_show);
            if(_call_show_end){
                /* notify listeners */
                _call_show_end(_show_end_obj);
            }
        }
    } else {
        log_debug("Show is not in progess: showstate=%#x.\n", _state);
    }
}

/*
 Load a show file.
 */
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
    if(STOPPED(_state))
        return 1;

    free_loaded_show();
    _live_show = show;
    return 0;
}

/*
 Only called from start_show()
 */
static void _start_show()
{
    spawn_joinable_pthread(&_show_pt, &next_step, NULL);
}

/*
 Start the show sequence.
 */
int start_show()
{
    if(NULL == _live_show) return DMX_SHOW_NOT_LOADED;
    if(RUNNING(_state)) return 0;

    pthread_mutex_lock(&_control_mutex);
    _state = OP_STATE_STARTING;

    _start_show();

    _state = OP_STATE_RUNNING;
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
    enum op_state lastState = _state;
    _state = OP_STATE_SKIPPING;

    if(RUNNING(lastState)){
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
    if(INTRANSIT(_state))
        return -1;

    pthread_mutex_lock(&_control_mutex);
    enum op_state last_state = _state;

    _state = OP_STATE_SKIPPING;

    if(RUNNING(last_state)) {
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
