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

// Globals
static FT_HANDLE    dmxDevice = 0;
//
static unsigned char *outputBuffer = 0;
//
static pthread_t dmx_writer_pt = 0;
//
static int writing = 0;
static int allowWrite = 0;

/*
   Zero out the write buffer then stop the DMX write thread.
 */
void stop_dmx()
{  
    if( !writing || !outputBuffer ){
        return;
    }

    printf("Stopping DMX transmission.\n");

    register int j;
    allowWrite = 0;

    if(dmxDevice) {
        for(j = 0; j < DMX_CHANNELS; j++) {
            outputBuffer[j] = 0;
        }
        usleep(500000);	
        writing=0;	
        pthread_join(dmx_writer_pt,NULL);
    }
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
        printf("Closed device.\n");
    }
    if(outputBuffer)
        free(outputBuffer);
}

/*
   This is the send thread.  Other threads will modify the
   write buffer while this thread just keeps sending it to the 
   output device.
 */
void *Write_Buffer(){

    DWORD dwBytesWritten;
    FT_STATUS ftStatus;
    useconds_t seconds;
    seconds=10000;
	
	if(!outputBuffer) pthread_exit(NULL);
	
    /* Write */
    while(writing && dmxDevice){
        FT_W32_SetCommBreak(dmxDevice);
        FT_W32_ClearCommBreak(dmxDevice);
        FT_Write(dmxDevice, 0, 1, &dwBytesWritten);
        if((ftStatus = FT_Write(dmxDevice, outputBuffer, DMX_CHANNELS, &dwBytesWritten)) != FT_OK) {
            printf("Error FT_Write(%d)\n", (int)ftStatus);
            break;
        }
        usleep(seconds);	
    }
    pthread_exit(NULL);
}

/*
   Update one channel with a new value.
 */
void update_channel(int ch, short val)
{
    if(!allowWrite || !outputBuffer) return;
    outputBuffer[ch] = val;
#ifdef _DMX_TRACE_OUTPUT
    printf("setting channel %d=%d\n", ch, val);
#endif    
}

/*
    Update all channels at once.
 */
void bulk_update(unsigned char* newVals)
{
    if(!allowWrite || !outputBuffer || !newVals) return;    
    memcpy(outputBuffer, newVals, DMX_CHANNELS);    
}

/*
   Get the current value for the channel.
 */
int get_channel_value(int ch){
    return outputBuffer ? outputBuffer[ch] : 0;
}

/*
   Connect to the first DMX device we find.
 */
int init_dmx()
{
    char * 	pcBufLD[MAX_DEVICES + 1];
    char 	cBufLD[MAX_DEVICES][64];

    FT_STATUS	ftStatus;
    //FT_HANDLE	dmxDevice;

    int iNumDevs = 0;
    int i;

    //init the device info buffers
    for(i = 0; i < MAX_DEVICES; i++) {
        pcBufLD[i] = cBufLD[i];
    }

    dmxDevice = 0;
    pcBufLD[MAX_DEVICES] = 0;

    ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
    printf("Number of devices(%d)\n", iNumDevs);

    if(0==iNumDevs || ftStatus != FT_OK) {
        fprintf(stderr, "Error: FT_ListDevices(%d)\n", (int)ftStatus);
        return DMX_INIT_NO_DEVICES;
    }

    for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
        printf("Device %d Serial Number - %s\n", i, cBufLD[i]);
    }
    i = 0;
    /* Setup */
    if((ftStatus = FT_OpenEx(cBufLD[i], FT_OPEN_BY_SERIAL_NUMBER, &dmxDevice)) != FT_OK){
        /* 
           This can fail if the ftdi_sio driver is loaded
           use lsmod to check this and rmmod ftdi_sio to remove
           also rmmod usbserial
         */
        fprintf(stderr, "Error FT_Open(%d), device: %d\n", (int)ftStatus, i);
        return DMX_INIT_OPEN_FAIL;
    }

    printf("Opened device %s\n", pcBufLD[0]);

    if((ftStatus = FT_SetBaudRate(dmxDevice, 250000)) != FT_OK) {
        fprintf(stderr, "Error FT_SetBaudRate(%d), cBufLD[0] = %s\n", (int)ftStatus, cBufLD[0]);
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

    outputBuffer = malloc(sizeof(char) * DMX_CHANNELS);
    memset(outputBuffer, 0, (sizeof(char)) * DMX_CHANNELS);

    return DMX_INIT_OK;
}

/*
   Start transmitting DMX messages to the device.
 */
void start_dmx()
{
    //If the DMX device is not initialized try
    // initializing then try running.  
    if(!dmxDevice){
        if(DMX_INIT_OK != init_dmx()){
            return;
        }
    }

    //If the DMX device is initialized correctly see
    // if we are already started.
    if(writing){
        return;
    }

    printf("Starting DMX transmission.\n");

    //Signal we are ready to run.
    writing = 1;

    pthread_create(&dmx_writer_pt, NULL, Write_Buffer, NULL);

    //We can now allow threads to write updates to the output buffer.
    allowWrite = 1;
}

/*
int main(int argc, char **argv)
{
    //signal(SIGINT, stop_dmx);		// trap ctrl-c call quit fn 
}
*/