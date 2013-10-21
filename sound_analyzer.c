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

#define DEFAULT_SLEEP_INTERVAL 100000

volatile static int _monitoring = 0;

static Movie *_movie = 0;
static pthread_t _monitor_thread = 0;
static pthread_t _callback_thread = 0;
static pthread_mutex_t _analyze_mutex = PTHREAD_MUTEX_INITIALIZER;

static void(*_listener_function)(void*, QTAudioFrequencyLevels*) = 0;
static void *_callback_ref = 0;

static QTAudioFrequencyLevels *_freq_results;

/*
 *   Structure for thread creation.
 *   Contains all the informtion for
 *   controlling a channel based on
 *   a movie/clip.
 */
typedef struct _monitor_data_t {
    const char *file_name;
    short refId;
    short frequency;
    channel_list_t channels;
    dmx_value_t dmxValue; // fix alignment
    double threshold;
    int flags;
    void(*callback)();
    void *callback_data;
} monitor_data_t;

static void(*_analyzer_function)(monitor_data_t*, QTAudioFrequencyLevels*) = 0;

/*
 * Print an analyzer setting to a show file.
 */
void print_analyzer(const analyzer_data_t *data, FILE *out)
{
    fprintf(out, "\tanalyzer {\n");
    fprintf(out, "\t\t file:%s;\n", data->movieFile);
    print_channel_list(data->dmxChannelList, out);
    fprintf(out, "\t\t threshold:%6.3f;\n", data->threshold);
    fprintf(out, "\t\t threshold_value:%d;\n", data->dmxValue);
    fprintf(out, "\t\t bands:%lu;\n", data->numberOfBandLevels);
    fprintf(out, "\t\t freq:%d;\n", data->frequency);
    fprintf(out, "\t\t type:%d;\n", data->flags);
    fprintf(out, "\t}\n");
}

inline static void free_monitor_data(monitor_data_t *mdata)
{
    /* don't free the file_name pointer... he's just borrowed */
    FREE_CHANNEL_LIST(mdata->channels);
    memset(mdata, 0, sizeof(monitor_data_t));
    free(mdata);
}
#define FREE_MONITOR_DATA(data) \
    free_monitor_data(data); \
    data = 0;

void free_analyzer_data(analyzer_data_t * adata)
{
    if(adata){
        free(adata->movieFile);
        FREE_CHANNEL_LIST(adata->dmxChannelList);
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
    if(_movie){
        GoToEndOfMovie(*_movie);
    }
}

/*
 Stop the analyzer background thread.
 */
void stop_analyze()
{
    pthread_mutex_lock(&_analyze_mutex);
    if(_monitoring){
        /* Flip the run flag for the monitoring thread */
        _monitoring = 0;
        if(_movie) StopMovie(*_movie);
    }
    pthread_mutex_unlock(&_analyze_mutex);

    if(_monitor_thread){
        pthread_join(_monitor_thread, NULL);
        _monitor_thread = 0;
    }
}

/*
    Thread function for performing callback when movie is finished.
*/
static void *do_callback(void *data_in)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.analyzer.callback");

    void(*callback_function)();
    callback_function = data_in;
    callback_function();
    EXIT_THREAD();
}

void register_self_as_freq_listener(void *callback_ref, void(*listenerFunction)(void*, QTAudioFrequencyLevels*))
{
    _listener_function = listenerFunction;
    _callback_ref = callback_ref;
}

void deregister_self_as_freqListner(void *callback_ref)
{
    _listener_function = 0;
    _callback_ref = 0;
}

/*
    Monitoring the frequency output of the movie and update the DMX
    channel(s) assigned to this effect.  This function trips the specified
    DMX channel when a peak threshold is reached.
*/
static void *monitor(void *data_in)
{

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.soundanalyzer");

    EnterMoviesOnThread(0);

    monitor_data_t *data = (monitor_data_t*)data_in;

    short movieId = data->refId;
    void(*callback)() = data->callback;

    /* start the channels at reset level. */
    update_channels(data->channels, CHANNEL_RESET);

    StartMovie(*_movie);

    log_debug("Playing movie: %s.\n", data->file_name);

    /* Don't stop unless the movie has stopped or the frequency buffer has vanished or
     * the user told us to.
     */
    while (_monitoring && !IsMovieDone(*_movie) && _freq_results) {

        OSStatus err = GetMovieAudioFrequencyLevels(*_movie, kQTAudioMeter_StereoMix, _freq_results);
        if(err){
            log_error( "Failed to produce frequency levels.  Cleaning up now.\n");
            goto cleanup;
        }

        if( _analyzer_function ){
            pthread_mutex_lock(&_analyze_mutex);
            if(_monitoring) _analyzer_function(data, _freq_results);
            pthread_mutex_unlock(&_analyze_mutex);
        }else{
            sleep(1);
        }

        if(_listener_function && _callback_ref)
            _listener_function(_callback_ref, _freq_results);

        MoviesTask(*_movie, 0);
    }

cleanup:

    log_debug( "Stopping movie.\n");
    /* Stop the movie first before we start to cleanup */
    if(_movie){
        StopMovie(*_movie);
    }

    pthread_mutex_lock(&_analyze_mutex);
        log_debug( "Cleaning up analyzer...\n");

        /* Reset the DMX channel. */
        update_channels(data->channels, CHANNEL_RESET);
        FREE_MONITOR_DATA(data);
        free(_freq_results);
        _freq_results = 0;
        CloseMovieFile(movieId);
        DisposeMovie(*_movie);
        free(_movie);
        _movie = 0;

        ExitMoviesOnThread();

        _monitoring = 0;

        log_debug( "Finished cleaning up analyzer.\n");
    pthread_mutex_unlock(&_analyze_mutex);

    /* Let our listener(s) know */
    if(callback){
        log_debug( "Notifying analyzer listeners.\n");
        spawn_joinable_pthread(&_callback_thread, do_callback, (void*)callback);
    } else {
        log_debug("No analyzer listeners.\n");
    }

    log_debug("Exiting analyzer thread.\n");
    EXIT_THREAD();
}

static void peak_monitor(monitor_data_t *data, QTAudioFrequencyLevels *freqs)
{
    Float32 value = freqs->level[data->frequency];
    if(value >= data->threshold ){
        update_channels(data->channels, data->dmxValue);
        usleep(DEFAULT_SLEEP_INTERVAL);
        update_channels(data->channels, CHANNEL_RESET);
    }else{
        usleep(DEFAULT_SLEEP_INTERVAL);
    }
}

static void follow_monitor(monitor_data_t *data, QTAudioFrequencyLevels *freqs)
{
    Float32 value = freqs->level[data->frequency];
    /* update the channel to the percentage of max based on the freq level. */
    dmx_value_t i = (dmx_value_t)(255 * value);
    update_channels(data->channels, i);
    usleep(DEFAULT_SLEEP_INTERVAL);
}

static void chase_monitor(monitor_data_t *data, QTAudioFrequencyLevels *freqs)
{
    static unsigned int lastChannel = 0;
    static dmx_value_t lastValue = 0;
    static useconds_t fade_interval = 2500;

    if(!data){
        lastChannel = 0;
        return;
    }
    unsigned int length = data->channels->length;
    if(lastChannel >= length) lastChannel = 0;
    dmx_channel_t ch = data->channels->channels[lastChannel];
    Float32 value = 0.0F;
    if(freqs) {
        value = freqs->level[data->frequency];
    }

    if(value > data->threshold){
        update_channel(ch, 0);
        ++lastChannel;
        if(lastChannel >= length) lastChannel = 0;
        ch = data->channels->channels[lastChannel];
        update_channel(ch, data->dmxValue);
        lastValue = data->dmxValue;
        usleep(DEFAULT_SLEEP_INTERVAL);
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
        usleep((useconds_t)500000);
        if(FSPathMakeRef((unsigned char*)fileName, &fsRef, false))
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
        log_error("movie err %d\n", (int)err);
        return ANALYZE_MOVIE_NOT_FOUND;
    }

    Boolean bWasChanged;
    short resId = 0;
    StringPtr resName = 0;

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
int start_analyze(const analyzer_data_t *data_in, void(*callback)())
{
    pthread_mutex_lock(&_analyze_mutex);
    if(_monitoring){
        pthread_mutex_unlock(&_analyze_mutex);
        return ANALYZE_IN_PROGRESS;
    }

    UInt32 numberOfChannels = 2;
    UInt32 numberOfBandLevels;

    numberOfBandLevels = data_in->numberOfBandLevels;

    _freq_results = 0;
    OSStatus err;

    const unsigned char* file_name = (unsigned char *) data_in->movieFile;
    short refId = 0;
    if(open_movie_file(file_name,&_movie, &refId)){
        pthread_mutex_unlock(&_analyze_mutex);
        return ANALYZE_FILE_NOT_FOUND;
    }

    SetMovieAudioFrequencyMeteringNumBands(*_movie, kQTAudioMeter_StereoMix, &numberOfBandLevels);

    _freq_results = malloc(offsetof(QTAudioFrequencyLevels,
                                  level[numberOfBandLevels * numberOfChannels]));
    if (!_freq_results) {
        /* TODO clean up */
        err = memFullErr;
        pthread_mutex_unlock(&_analyze_mutex);
        return err;
    }

    _freq_results->numChannels = numberOfChannels;
    _freq_results->numFrequencyBands = numberOfBandLevels;

    /*
     * Repackage our structure to send to the monitor thread with
     * a few pieces of extra data.
     */
    monitor_data_t *data = malloc(sizeof(monitor_data_t));

    data->channels = copy_channel_list(data_in->dmxChannelList);

    data->flags = data_in->flags;
    data->frequency = data_in->frequency;
    data->threshold = data_in->threshold;
    data->dmxValue = data_in->dmxValue;
    data->refId = refId;
    data->file_name = data_in->movieFile;
    data->callback = callback;

    switch(data_in->flags){
        case analyzeMonitorFollow:
            _analyzer_function = follow_monitor;
            break;
        case analyzeMonitorPeak:
            _analyzer_function = peak_monitor;
            break;
        case analyzeMonitorChase:
            _analyzer_function = chase_monitor;
            break;
        default:
            _analyzer_function = 0;
            break;
    }
    _monitoring = 1;
    if(spawn_joinable_pthread(&_monitor_thread, monitor, (void*)data) != 0){
        log_error("Failed to start thread for sound analyzer.\n");
    }

    pthread_mutex_unlock(&_analyze_mutex);

    return ANALYZE_OK;
}
