/*
 *  sound_analyzer.c
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/15/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include "sound_analyzer.h"
#include "dmx_controller.h"
#include <pthread.h>
#include <string.h>

#define SLEEP_INTERVAL 100000

//
static Movie *movie = 0; 
//
static pthread_t monitor_thread = 0;
static pthread_t callback_thread = 0;
//
volatile static int monitoring = 0;
//
static pthread_mutex_t analyze_mutex = PTHREAD_MUTEX_INITIALIZER;
//
static void(*_listenerFunction)(void*, QTAudioFrequencyLevels*) = 0;
//
static void *_callbackRef = 0;
//
static QTAudioFrequencyLevels *freqResults;

/*
    Structure for thread creation.
    Contains all the informtion for
    controlling a channel based on
    a movie/clip.
*/
typedef struct _monitor_data_t {
    short refId;
    short frequency;
    channel_list_t dmxChannelList;
    dmx_value_t dmxValue; // fix alignment
    Float32 threshold;
    int flags;
    void(*callback)();
    void *callback_data;
} monitor_data_t;

//
static void(*analyzer_function)(monitor_data_t*, QTAudioFrequencyLevels*) = 0;
//
void free_monitor_data(monitor_data_t *mdata)
{
    DELETE_CHANNEL_LIST(mdata->dmxChannelList);    
    memset(mdata, 0, sizeof(monitor_data_t));
    free(mdata);
}
#define FREE_MONITOR_DATA(data) \
    free_monitor_data(data); \
    data = 0;
//
void free_analyzer_data(analyzer_data_t * adata)
{
    if(adata){
        free(adata->movieFile);
        DELETE_CHANNEL_LIST(adata->dmxChannelList);
        memset(adata, 0, sizeof(analyzer_data_t));
        free(adata);
        adata = 0;
    }
}

/*
 Stop the movie if one is playing.  This function allows
 the controller functions to keep playing the show.
 */
void skip_movie()
{
    if(movie){
        GoToEndOfMovie(*movie);
    }
}

/*
 Stop the analyzer background thread.
 */
void stop_analyze()
{
    pthread_mutex_lock(&analyze_mutex);
    if(monitoring){
        //Flip the run flag for the monitoring thread
        monitoring = 0;
        if(movie) StopMovie(*movie);
    }
    pthread_mutex_unlock(&analyze_mutex);
    //Wait for the monitoring thread to exit;
    if(monitor_thread){
        pthread_join(monitor_thread, NULL);
        monitor_thread = 0;
    }
}

/*
    Thread function for performing callback when movie is finished.
*/
void *do_callback(void *data_in)
{
    void(*callback_function)();
    callback_function = data_in;
    callback_function();
    pthread_exit(NULL);
}

void registerSelfAsFreqListener(void *callbackRef, void(*listenerFunction)(void*, QTAudioFrequencyLevels*))
{
    _listenerFunction = listenerFunction;
    _callbackRef = callbackRef;
}

void deregisterSelfAsFreqListner(void *callbackRef)
{
    _listenerFunction = 0;
    _callbackRef = 0;
}

/*
    Monitoring the frequency output of the movie and update the DMX 
    channel(s) assigned to this effect.  This function trips the specified
    DMX channel when a peak threshold is reached.
*/
void *monitor(void *data_in)
{

    EnterMoviesOnThread(0);
    
    monitor_data_t *data = (monitor_data_t*)data_in;
           
    short movieId = data->refId;
    void(*callback)() = data->callback;

    //start the channels at reset level.
    update_channels(data->dmxChannelList, CHANNEL_RESET);    

    StartMovie(*movie);
    //Don't stop unless the movie has stopped or the frequency buffer has vanished or
    // the user told us to.
    while (monitoring && !IsMovieDone(*movie) && freqResults) {
        
        OSStatus err = GetMovieAudioFrequencyLevels(*movie, kQTAudioMeter_StereoMix, freqResults);
        if(err){
            goto cleanup;
        }
        
        if( analyzer_function ){
            pthread_mutex_lock(&analyze_mutex);
            if(monitoring) analyzer_function(data, freqResults);
            pthread_mutex_unlock(&analyze_mutex);
        }else{
            sleep(1);
        }
        
        if(_listenerFunction && _callbackRef)
            _listenerFunction(_callbackRef, freqResults);
        #ifdef _CLI_MAIN
        MoviesTask(*movie, 0);
        #endif
    }
    
cleanup:
    //Stop the movie first before we start to cleanup
    if(movie){// && !IsMovieDone(*movie)){
        StopMovie(*movie);           
    }    
    //we aren't playing anymore.
    pthread_mutex_lock(&analyze_mutex);
        monitoring = 0;   
        //Reset the DMX channel.
        update_channels(data->dmxChannelList, CHANNEL_RESET);  
        //Free the input
        FREE_MONITOR_DATA(data);
        free(freqResults);
        freqResults = 0;
    pthread_mutex_unlock(&analyze_mutex);
    //Free up these resources now.
    CloseMovieFile(movieId);
    DisposeMovie(*movie);
    free(movie);
    movie = 0;
    
    //Let our listener(s) know
    if(callback){// && monitoring){
        pthread_create(&callback_thread, NULL, do_callback, (void*)callback);
    } 
    
    //Knock down QT and quit.
    ExitMoviesOnThread();
    pthread_exit(NULL);
}

void peak_monitor(monitor_data_t *data, QTAudioFrequencyLevels *freqs)
{
    Float32 value = freqs->level[data->frequency];
    if(value >= data->threshold ){
        update_channels(data->dmxChannelList, data->dmxValue); 
        usleep(SLEEP_INTERVAL);
        update_channels(data->dmxChannelList, CHANNEL_RESET);
    }else{ 
        usleep(SLEEP_INTERVAL); 
    }
}

void follow_monitor(monitor_data_t *data, QTAudioFrequencyLevels *freqs)
{
    Float32 value = freqs->level[data->frequency];
    /* update the channel to the percentage of max based on the freq level. */
    int i = 255 * value;
    update_channels(data->dmxChannelList, i);
    usleep(SLEEP_INTERVAL);  
}

void chase_monitor(monitor_data_t *data, QTAudioFrequencyLevels *freqs)
{
    static int lastChannel = 0;
    static dmx_value_t lastValue = 0;
    static useconds_t fade_interval = 2500;
    
    if(!data){
        lastChannel = 0;
        return;
    }
    //
    unsigned int length = data->dmxChannelList->length;
    if(lastChannel >= length) lastChannel = 0;
    dmx_channel_t ch = data->dmxChannelList->channels[lastChannel];
    Float32 value = 0.0;
    if(freqs)
        value = freqs->level[data->frequency];
    //
    if(value > data->threshold){
        update_channel(ch, 0);
        ++lastChannel;
        if(lastChannel >= length) lastChannel = 0;
        ch = data->dmxChannelList->channels[lastChannel];
        update_channel(ch, data->dmxValue);
        lastValue = data->dmxValue;
        usleep(SLEEP_INTERVAL);
    } else {
        if(lastValue > 0){
            lastValue = lastValue > 0 ? --lastValue : 0;
            update_channel(ch, lastValue);
        }
        usleep(fade_interval);//need a fade param
    }
}

int open_movie_file(const unsigned char *fileName, Movie **newMovie, short *refId)
{
    OSStatus err;
    FSRef fsRef;
    
    err = FSPathMakeRef((unsigned char*)fileName, &fsRef, false);
    if(err){
        return ANALYZE_FILE_NOT_FOUND;
    }
    
    FSCatalogInfoBitmap pInfo = 0;
    FSSpec file;
    err = FSGetCatalogInfo(&fsRef, pInfo,NULL, NULL, &file, NULL);
    if(err){
        return ANALYZE_FILE_NOT_FOUND;
    }
  
    err = OpenMovieFile(&file, refId, fsRdPerm);
    if(err){
        printf("movie err %d\n", (int)err);
        return ANALYZE_MOVIE_NOT_FOUND;
    }
    
    Boolean bWasChanged;
    short resId = 0;
    StringPtr resName = 0;
    
    //Make some space for our new movie.
    if(!*newMovie){
        *newMovie = malloc(sizeof(Movie));
    }
    err = NewMovieFromFile(*newMovie, *refId, &resId, resName, newMovieActive, &bWasChanged);
    if(err){
        free(*newMovie);
        *newMovie = 0;
        return ANALYZE_MOVIE_CREATE_FAILED;
    }  
    return 0;  
}

/*
    Initialize and start the audio thread.
*/
int start_analyze(analyzer_data_t *data_in, void(*callback)())
{
    pthread_mutex_lock(&analyze_mutex);
    if(monitoring){
        pthread_mutex_unlock(&analyze_mutex);
        return ANALYZE_IN_PROGRESS;
    }
    monitoring = 1;
    pthread_mutex_unlock(&analyze_mutex);
    
    UInt32 numberOfChannels = 2; 
    UInt32 numberOfBandLevels;
    
    numberOfBandLevels = data_in->numberOfBandLevels;
    
    freqResults = 0;
    OSStatus err;
 
    const unsigned char* fileName = (unsigned char *) data_in->movieFile;
    short refId = 0;
    if(open_movie_file(fileName,&movie, &refId)){
		return ANALYZE_FILE_NOT_FOUND;
	}

    SetMovieAudioFrequencyMeteringNumBands(*movie, kQTAudioMeter_StereoMix, &numberOfBandLevels);
    
    //Create some space for the frequency buffer.
    freqResults = malloc(offsetof(QTAudioFrequencyLevels,
                                  level[numberOfBandLevels * numberOfChannels]));
    if (!freqResults) {
        //TODO clean up
        err = memFullErr;
        return err;    
    }
  
    freqResults->numChannels = numberOfChannels;
    freqResults->numFrequencyBands = numberOfBandLevels;
    
    // Repackage our structure to send to the monitor thread so
    // that our caller can cleanup their allocations immediately and we
    // can cleanup this new allocation in the pthread once it makes
    // a copy.
    monitor_data_t *data = malloc(sizeof(monitor_data_t));
    
    data->dmxChannelList = copy_channel_list(data_in->dmxChannelList);
   
    data->flags = data_in->flags;
    data->frequency = data_in->frequency;
    data->threshold = data_in->threshold;  
    data->dmxValue = data_in->dmxValue;
    data->refId = refId;    
    data->callback = callback;
   
    switch(data_in->flags){
        case analyzeMonitorFollow:
            analyzer_function = follow_monitor;
            break;
        case analyzeMonitorPeak:
            analyzer_function = peak_monitor;
            break;
        case analyzeMonitorChase:
            analyzer_function = chase_monitor;
            break;
    }
    
    pthread_create(&monitor_thread, NULL, monitor, (void*)data);       
    return ANALYZE_OK;
}

