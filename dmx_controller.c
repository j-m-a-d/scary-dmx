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

#define MAX_DEVICES		1

/* Globals */
static FT_HANDLE    dmxDevice = 0;

static dmx_value_t outputBuffer[DMX_CHANNELS];

static pthread_t dmx_writer_pt = 0;
static pthread_mutex_t dmx_mutex = PTHREAD_MUTEX_INITIALIZER;

volatile static int writing = 0;

/*
   Zero out the write buffer then stop the DMX write thread.
 */
void stop_dmx()
{
    if( !writing || !outputBuffer ){
        return;
    }

    log_info("Stopping DMX transmission.\n");

    pthread_mutex_lock(&dmx_mutex);
    
    memset(outputBuffer, 0, DMX_CHANNELS * sizeof(dmx_value_t));
    if(dmxDevice) {
        usleep(500000);	
        writing=0;	
        pthread_join(dmx_writer_pt,NULL);
    }

    pthread_mutex_unlock(&dmx_mutex);
}

/*
   Stop the current DMX transmission if running
   and free up the resources for the device.
 */
void destroy_dmx()
{
    stop_dmx();
    if(dmxDevice){
        FT_Close(dmxDevice);
        dmxDevice = 0;
        log_info("Closed device.\n");
    }
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

	if(!outputBuffer) pthread_exit(NULL);

    /* Write */
    while(writing && dmxDevice){
        FT_W32_SetCommBreak(dmxDevice);
        FT_W32_ClearCommBreak(dmxDevice);
        FT_Write(dmxDevice, 0, 1, &dwBytesWritten);
        if((ftStatus = FT_Write(dmxDevice, outputBuffer, DMX_CHANNELS, &dwBytesWritten)) != FT_OK) {
            log_info("Error FT_Write(%d)\n", (int)ftStatus);
            writing = 0;
            break;
        }
        usleep(seconds);	
    }
    EXIT_THREAD();
}

/*
   Update one channel with a new value.
 */
void update_channel(dmx_channel_t ch, dmx_value_t val)
{
    pthread_mutex_lock(&dmx_mutex);

#ifdef _DMX_TRACE_OUTPUT
    if(!allowWrite || ch >= DMX_CHANNELS ){
        log_error( "Incorrect state for dmx channel update.  allow write: %d, output buffer address: %ld, channel: %d, value:%d\n", allowWrite, (long)&outputBuffer, ch, val);
        pthread_mutex_unlock(&dmx_mutex);
        return;
    }
    if(!ch){
        log_error( "Broadcast channel selected.\n");
    }
    log_error( "Ch: %d -> %d\n", ch, val);
#endif

    //if(allowWrite){
        outputBuffer[ch] = val;
    //}

    pthread_mutex_unlock(&dmx_mutex);
}

/*
    Update multiple channels with a new value.
 */
void update_channels(channel_list_t channelList, dmx_value_t val)
{
    pthread_mutex_lock(&dmx_mutex);

    /*
    if(!allowWrite){
        pthread_mutex_unlock(&dmx_mutex);
        return;
    }
     */

    dmx_channel_t *tmp;
    tmp = channelList->channels;
    while(*tmp){
#ifdef _DMX_TRACE_OUTPUT
        if(*tmp >= DMX_CHANNELS ){
            log_error( "Incorrect state for dmx channel update.  allow write: %d, output buffer address: %ld, channel: %d, value:%d\n", allowWrite, (long)&outputBuffer, *tmp, val);
            pthread_mutex_unlock(&dmx_mutex);
            return;
        }
        if(!*tmp){/* can't happen */
            log_error( "Broadcast channel selected.\n");
        }
        log_error( "Multichannel : %d\n", val);
#endif
        outputBuffer[*tmp] = val;
        tmp++;
    }

    pthread_mutex_unlock(&dmx_mutex);
}

/*
    Update all channels at once.
 */
void bulk_update(unsigned char* newVals)
{
    pthread_mutex_lock(&dmx_mutex);

    if(newVals)
        memcpy(outputBuffer, newVals, DMX_CHANNELS);

    pthread_mutex_unlock(&dmx_mutex);
}

/*
   Get the current value for the channel.
 */
dmx_value_t get_channel_value(int ch){
    return outputBuffer ? outputBuffer[ch] : 0;
}

/*
   Get a copy of a portion of the DMX output buffer
 */
void get_channel_buffer(dmx_value_t *buffer, int offset, int num_channels)
{
    memcpy(buffer, outputBuffer + offset, sizeof(dmx_value_t) * (unsigned int)num_channels);
}

/*
   Connect to the first DMX device we find.
 */
int init_dmx()
{
    char * 	pcBufLD[MAX_DEVICES + 1];
    char 	cBufLD[MAX_DEVICES][64];

    FT_STATUS	ftStatus;

    int iNumDevs = 0;
    int i;

    /* Init the device info buffers */
    for(i = 0; i < MAX_DEVICES; i++) {
        pcBufLD[i] = cBufLD[i];
    }

    dmxDevice = 0;
    pcBufLD[MAX_DEVICES] = 0;

    ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
    log_info("Number of devices(%d)\n", iNumDevs);

    if(0==iNumDevs || ftStatus != FT_OK) {
        log_error( "Error: FT_ListDevices(%d)\n", (int)ftStatus);
        return DMX_INIT_NO_DEVICES;
    }

    for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
        log_info("Device %d Serial Number - %s\n", i, cBufLD[i]);
    }

    i = 0;

    /* Setup */
    if((ftStatus = FT_OpenEx(cBufLD[i], FT_OPEN_BY_SERIAL_NUMBER, &dmxDevice)) != FT_OK){
        log_error( "Error FT_Open(%d), device: %d\n", (int)ftStatus, i);
        return DMX_INIT_OPEN_FAIL;
    }

    log_info("Opened device %s\n", pcBufLD[0]);

    if((ftStatus = FT_SetBaudRate(dmxDevice, 250000)) != FT_OK) {
        log_error( "Error FT_SetBaudRate(%d), cBufLD[0] = %s\n", (int)ftStatus, cBufLD[0]);
        return DMX_INIT_SET_BAUD_FAIL;
    }

    FTDCB ftDCB;
    if (FT_W32_GetCommState(dmxDevice,&ftDCB)) {
        // FT_W32_GetCommState ok, device state is in ftDCB
        ftDCB.BaudRate = 250000;
        ftDCB.Parity = FT_PARITY_NONE;
        ftDCB.StopBits = FT_STOP_BITS_2;
        ftDCB.ByteSize = FT_BITS_8;
        ftDCB.fOutX = 0;
        ftDCB.fInX = 0;
        ftDCB.fErrorChar = 0;
        ftDCB.fBinary = 1;
        ftDCB.fRtsControl = 0;
        ftDCB.fAbortOnError = 0;

        if (!FT_W32_SetCommState(dmxDevice,&ftDCB)) {
            return DMX_INIT_SET_DATA_FLOW_FAIL;
        }
    }	

    FT_W32_PurgeComm(dmxDevice,FT_PURGE_TX | FT_PURGE_RX);

    memset(outputBuffer, 0, (sizeof(char)) * DMX_CHANNELS);

    return DMX_INIT_OK;
}

/*
   Start transmitting DMX messages to the device.
 */
void start_dmx()
{
    /* If the DMX device is not initialized try
     *  initializing then try running.
     */
    pthread_mutex_lock(&dmx_mutex);
        /* If the DMX device is initialized correctly see
         * if we are already started.
         */
        if(writing || dmxDevice){
            pthread_mutex_unlock(&dmx_mutex);
            return;
        }
        destroy_dmx();
        if(DMX_INIT_OK != init_dmx()){
            pthread_mutex_unlock(&dmx_mutex);
            return;
        }
        spawn_joinable_pthread(&dmx_writer_pt, write_buffer, NULL);
        /* TODO check result ^^ */
        log_error( "Starting DMX transmission.\n");
        /* We can now allow threads to write updates to the output buffer. */
        writing = 1;
    pthread_mutex_unlock(&dmx_mutex);
}

