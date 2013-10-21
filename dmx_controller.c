/*
 *  dmxcontroller.c
 *  Effects Controller
 *
 *  Created by Jason DiPrinzio on 9/13/08.
 *  Copyright 2008 Inspirotech Inc. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "ftd2xx.h"
#include "dmx_controller.h"

#define MAX_DEVICES     1

/* Globals */
static FT_HANDLE _dmx_device = 0;

static dmx_value_t _output_buffer[DMX_CHANNELS];

static pthread_t _dmx_writer_pt = 0;
static pthread_mutex_t _dmx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _wait_cond = PTHREAD_COND_INITIALIZER;

volatile static enum op_state _dmxstate = OP_STATE_INITIALIZING;

/*
   Zero out the write buffer then stop the DMX write thread.
 */
void stop_dmx()
{
    if( INTRANSIT(_dmxstate) || !_output_buffer ){
        return;
    }
    pthread_mutex_lock(&_dmx_mutex);
        if(!RUNNING(_dmxstate)){
            pthread_mutex_unlock(&_dmx_mutex);
            return;
        }
        _dmxstate = OP_STATE_STOPPING;
        log_info("Stopping DMX transmission.\n");

        memset(_output_buffer, 0, DMX_CHANNELS * sizeof(dmx_value_t));

        pthread_join(_dmx_writer_pt,NULL);
        _dmxstate = OP_STATE_STOPPED;
    pthread_mutex_unlock(&_dmx_mutex);
}

static void close_device()
{
    if(_dmx_device){
        FT_Close(_dmx_device);
        _dmx_device = 0;
        log_info("Closed device.\n");
    }
}

/*
   Stop the current DMX transmission if running
   and free up the resources for the device.
 */
void destroy_dmx()
{
    stop_dmx();
    close_device();
}

/*
   This is the send thread.  Other threads will modify the
   write buffer while this thread just keeps sending it to the
   output device.
 */
void *write_buffer(){

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    PTHREAD_SETNAME("scarydmx.dmxwriter");

    DWORD dwBytesWritten;
    FT_STATUS ftStatus;
    useconds_t seconds;
    seconds=20000;

    if(!_output_buffer) pthread_exit(NULL);

    /* Wait until synced */
    while(OP_STATE_STARTING == _dmxstate) {
        pthread_mutex_lock(&_dmx_mutex);
        pthread_cond_wait(&_wait_cond, &_dmx_mutex);
        pthread_mutex_unlock(&_dmx_mutex);
    }

    /* Write */
    while(RUNNING(_dmxstate) && _dmx_device){
        FT_SetBreakOn(_dmx_device);
        FT_SetBreakOff(_dmx_device);

        if((ftStatus = FT_Write(_dmx_device, _output_buffer, DMX_CHANNELS, &dwBytesWritten)) != FT_OK) {
            log_error("Error FT_Write(%d)\n", (int)ftStatus);
            pthread_mutex_lock(&_dmx_mutex);
                if(OP_STATE_RUNNING == _dmxstate) {
                    close_device();
                    _dmxstate = OP_STATE_STOPPED;
                }
            pthread_mutex_unlock(&_dmx_mutex);
            break;
        }
        usleep(seconds);
    }
    log_debug("DMX writer thread exiting.\n");
    EXIT_THREAD();
}

/*
   Update one channel with a new value.
 */
void update_channel(dmx_channel_t ch, dmx_value_t val)
{
    pthread_mutex_lock(&_dmx_mutex);
        _output_buffer[ch] = val;
    pthread_mutex_unlock(&_dmx_mutex);
}

/*
    Update multiple channels with a new value.
 */
void update_channels(const channel_list_t channelList, dmx_value_t val)
{
    if( !channelList ) return;
    pthread_mutex_lock(&_dmx_mutex);
        dmx_channel_t *tmp;
        tmp = channelList->channels;
        while(*tmp){
            _output_buffer[*tmp] = val;
            tmp++;
        }
    pthread_mutex_unlock(&_dmx_mutex);
}

/*
    Update all channels at once.
 */
void bulk_update(const unsigned char* new_vals)
{
    pthread_mutex_lock(&_dmx_mutex);

    if(new_vals)
        memcpy(_output_buffer, new_vals, DMX_CHANNELS);

    pthread_mutex_unlock(&_dmx_mutex);
}

/*
   Get the current value for the channel.
 */
dmx_value_t get_channel_value(dmx_channel_t ch){
    return _output_buffer ? _output_buffer[ch] : 0;
}

/*
   Get a copy of a portion of the DMX output buffer
 */
void get_channel_buffer(const dmx_value_t *buffer, const int offset, const int num_channels)
{
    memcpy((void*)buffer, _output_buffer + offset, sizeof(dmx_value_t) * (unsigned int)num_channels);
}

static void describe_devices(FT_DEVICE_LIST_INFO_NODE *device_info)
{
    DWORD num_devices;
    FT_STATUS ftStatus;

    FT_CreateDeviceInfoList(&num_devices);

    if(num_devices > 0) {
        ftStatus = FT_GetDeviceInfoList(device_info, &num_devices);
        if (ftStatus == FT_OK) {
            for (unsigned int i = 0; i < num_devices; i++) {
                log_debug("Devivce #%d:\n",i);
                log_debug("-id: 0x%x\n",device_info[i].ID);
                log_debug("-location: 0x%x\n",device_info[i].LocId);
                log_debug("-serial num: %s\n",device_info[i].SerialNumber);
                log_debug("-flags: 0x%x\n",device_info[i].Flags);
                log_debug("-type: 0x%x\n",device_info[i].Type);
                log_debug("-handle: 0x%x\n",(unsigned int)device_info[i].ftHandle);
                log_debug("-description: %s\n",device_info[i].Description);
            }
        }
    }
}

/*
 * Connect to the first DMX device we find.
 */
int init_dmx()
{
    char *  pcBufLD[MAX_DEVICES + 1];
    memset(pcBufLD, 0, (sizeof(char) * MAX_DEVICES) + 1);

    char    cBufLD[MAX_DEVICES][64];
    memset(cBufLD, 0, sizeof(cBufLD));

    FT_STATUS   ftStatus;

    int num_devices = 0;

    /* Init the device info buffers */
    for(int i = 0; i < MAX_DEVICES; i++) {
        pcBufLD[i] = cBufLD[i];
    }

    _dmx_device = 0;
    pcBufLD[MAX_DEVICES] = 0;

    ftStatus = FT_ListDevices(pcBufLD, &num_devices, FT_LIST_ALL);
    log_info("Number of devices(%d)\n", num_devices);

    if(0==num_devices || ftStatus != FT_OK) {
        log_error( "Error: FT_ListDevices(%d)\n", (int)ftStatus);
        destroy_dmx();
        return DMX_INIT_NO_DEVICES;
    }

    FT_DEVICE_LIST_INFO_NODE device_info[num_devices];
    memset(device_info, 0, sizeof(FT_DEVICE_LIST_INFO_NODE) * (unsigned long)num_devices);
    describe_devices(device_info);

    if( !device_info[0].LocId ) {
        log_error("Error describe_devices failed to read dmx device(s)\n");
        destroy_dmx();
        return DMX_INIT_OPEN_FAIL;
    }

    /* Setup */
    if( (ftStatus = FT_OpenEx((void*)device_info[0].LocId, FT_OPEN_BY_LOCATION, &_dmx_device)) != FT_OK) {
        log_error( "Error FT_Open(%d), device: %d\n", (int)ftStatus, 1);
        destroy_dmx();
        return DMX_INIT_OPEN_FAIL;
    }

    log_info("Opened device %s\n", pcBufLD[0]);

    if((ftStatus = FT_SetBaudRate(_dmx_device, 250000)) != FT_OK) {
        log_error( "Error FT_SetBaudRate(%d), cBufLD[0] = %s\n", (int)ftStatus, cBufLD[0]);
        destroy_dmx();
        return DMX_INIT_SET_BAUD_FAIL;
    }

    if( (ftStatus = FT_SetDataCharacteristics(_dmx_device, FT_BITS_8, FT_STOP_BITS_2, FT_PARITY_NONE) != FT_OK)){
        log_error("Set data profile failed - (%d)\n", (int)ftStatus);
    }
    if( (ftStatus = FT_SetFlowControl(_dmx_device, FT_FLOW_NONE, 0, 0) != FT_OK)){
        log_error("Set flow control failed - (%d)\n", (int)ftStatus);
    }
    if( (ftStatus = FT_Purge(_dmx_device,FT_PURGE_TX | FT_PURGE_RX) != FT_OK)){
        log_error("Purge device failed - (%d)\n", (int)ftStatus);
    }

    memset(_output_buffer, 0, (sizeof(char)) * DMX_CHANNELS);

    return DMX_INIT_OK;
}

/*
   Start transmitting DMX messages to the device.
 */
void start_dmx()
{
    /*
     * If the DMX device is not initialized try
     * initializing then try running.
     */
    pthread_mutex_lock(&_dmx_mutex);
        /*
         * If the DMX device is initialized correctly see
         * if we are already started.
         */
        if(!STOPPED(_dmxstate)) {
            pthread_mutex_unlock(&_dmx_mutex);
            return;
        }
        _dmxstate = OP_STATE_STARTING;

        log_info( "Starting DMX transmission.\n");
        int result;
        if( (result = spawn_joinable_pthread(&_dmx_writer_pt, write_buffer, NULL)) ) {
            log_warn("Failed to spawn thread for dmx writer [%d]\n", result);
            _dmxstate = OP_STATE_STOPPED;
        } else {
            /* We can now allow threads to write updates to the output buffer. */
            _dmxstate = OP_STATE_RUNNING;
        }
        pthread_cond_broadcast(&_wait_cond);

    pthread_mutex_unlock(&_dmx_mutex);
}

