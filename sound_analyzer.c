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
//
static Movie *movie = 0; 
//
static pthread_t MonitorThread = 0;
static pthread_t CallbackThread = 0;
//
static int monitoring = 0;
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
    channel_list_t dmxChannelList;
    int dmxValue;
    short frequency;
    Float32 threshold;
    int flags;
    void(*callback)();
    void *callback_data;
} monitor_data_t;

void free_analyzer_data(analyzer_data_t * adata)
{
    if(adata){
        free(adata->movieFile);
        free(adata->dmxChannelList);
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
    if(MonitorThread)
        pthread_join(MonitorThread, NULL);    
}

/*
    Thread function for performing callback when movie is finished.
*/
void *do_callback(void *data_in)
{
    void(*callback_function)() __attribute__((pascal));
    callback_function = data_in;
    callback_function();
    pthread_exit(NULL);
}

void registerSelfAsFreqListener(void *callbackRef, void(*listenerFunction)(void*, QTAudioFrequencyLevels*))
{
    _listenerFunction = listenerFunction;
    _callbackRef = callbackRef;
}

/*
 Update all the channels set for this analyzer to the specified value.
 */
static void update_channel_list(channel_list_t data, int value)
{
    int *tmp = data;
    while(*tmp){
        update_channel(*tmp, value);
        tmp++;
    }
}

/*
    Monitoring the frequency output of the movie and update the DMX 
    channel(s) assigned to this effect.  This function trips the specified
    DMX channel when a peak threshold is reached.
*/
void *peak_monitor(void *data_in)
{

    EnterMoviesOnThread(0);
    
    monitor_data_t data = *(monitor_data_t*)data_in;
        
    free(data_in);
    data_in = 0;
    
    short movieId = data.refId;
    void(*callback)() = data.callback;

    //start the channels at reset level.
    update_channel_list(data.dmxChannelList, CHANNEL_RESET);    

    //Start the show.
    StartMovie(*movie);
    //Don't stop unless the movie has stopped or the frequency buffer has vanished or
    // the user told us to.
    while (monitoring && !IsMovieDone(*movie) && freqResults) {
        
        OSStatus err = GetMovieAudioFrequencyLevels(*movie, kQTAudioMeter_StereoMix, freqResults);
        if(err){
            goto cleanup;
        }
        Float32 value = freqResults->level[data.frequency];
        if(value >= data.threshold ){
            update_channel_list(data.dmxChannelList, data.dmxValue);  
            usleep(100000);  
            update_channel_list(data.dmxChannelList, CHANNEL_RESET);
        }  
        if(_listenerFunction && _callbackRef)
            _listenerFunction(_callbackRef, freqResults);
        usleep(1000);   
    }
    
cleanup:
    //Stop the movie first before we start to cleanup
    if(movie){// && !IsMovieDone(*movie)){
        StopMovie(*movie);           
    }

    //Free up these resources now.
    free(freqResults);
    freqResults = 0;
    CloseMovieFile(movieId);
    DisposeMovie(*movie);
    free(movie);
    movie = 0;
    
    //Reset the DMX channel.
    update_channel_list(data.dmxChannelList, CHANNEL_RESET);
    
    //Let our listener(s) know
    if(callback && monitoring){
        pthread_create(&CallbackThread, NULL, do_callback, (void*)callback);
    }   
    
    //we aren't playing anymore.
    pthread_mutex_lock(&analyze_mutex);
    monitoring = 0;
    pthread_mutex_unlock(&analyze_mutex);
    
    //Knock down QT and quit.
    ExitMoviesOnThread();
    pthread_exit(NULL);
}

/*
    Monitoring the frequency output of the movie and update the DMX 
    channel(s) assigned to this effect.  This function trips the specified
    DMX channel by following the frequency level.
*/
void *follow_monitor(void* data_in)
{
    //This thread needs to tell QuickTime that it is going to play movies.
    // We probably won't get any interference for now since this is the only
    // thread that actually plays a movie.
    EnterMoviesOnThread(0);
    
    monitor_data_t data = *(monitor_data_t*)data_in;
        
    free(data_in);
    data_in = 0;
    
    short movieId = data.refId;
    void(*callback)() = data.callback;

    //start the channel at reset level.
    update_channel_list(data.dmxChannelList, CHANNEL_RESET);    

    //Start the show.
    StartMovie(*movie);
    //Don't stop unless the movie has stopped or the frequency buffer has vanished or
    // the user told us to.
    while (monitoring && !IsMovieDone(*movie) && freqResults) {
        
        OSStatus err = GetMovieAudioFrequencyLevels(*movie, kQTAudioMeter_StereoMix, freqResults);
        if(err){
            goto cleanup;
        }

        //Grab the freq level.
        Float32 value = freqResults->level[data.frequency];
        //update the channel to the percentage of max based on the freq level.
        update_channel_list(data.dmxChannelList, (255 * value));
        if(_listenerFunction && _callbackRef)
            _listenerFunction(_callbackRef, freqResults);
        usleep(1000);   
    }
    
cleanup:

    //Stop the movie first before we start to cleanup
    if(movie){// && !IsMovieDone(*movie)){
        StopMovie(*movie);           
    }
    
    //Free up these resources now.
    free(freqResults);
    freqResults = 0;
    CloseMovieFile(movieId);
    DisposeMovie(*movie);
    free(movie);
    movie = 0;
    
    //Reset the DMX channel.
    update_channel_list(data.dmxChannelList, CHANNEL_RESET);
      
    //Let our listener(s) know
    if(callback && monitoring){
        pthread_create(&CallbackThread, NULL, do_callback, (void*)callback);
    }
    //Signal we aren't playing anymore.
    pthread_mutex_lock(&analyze_mutex);
    monitoring = 0;
    pthread_mutex_unlock(&analyze_mutex);
    
    //Knock down QT and quit.
    ExitMoviesOnThread();    
    pthread_exit(NULL);
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
    //FIXME copy the data not just the pointer
    data->dmxChannelList = data_in->dmxChannelList;
    data->dmxValue = data_in->dmxValue;
    data->flags = data_in->flags;
    data->frequency = data_in->frequency;
    data->threshold = data_in->threshold;  
      
    data->refId = refId;    
    data->callback = callback;
   
    if(data_in->flags && analyzeMonitorFollow){
        pthread_create(&MonitorThread, NULL, follow_monitor, (void*)data);   
    } else {
        pthread_create(&MonitorThread, NULL, peak_monitor, (void*)data);   
    }
    
    return ANALYZE_OK;
}



