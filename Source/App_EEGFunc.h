/*
 * App_EEGFunc.h : EEG application Function Model header file
 * Written by Chenm
 */

#ifndef APP_EEGFUNC_H
#define APP_EEGFUNC_H

#include "hal_types.h"

extern void EEGFunc_Init(uint8 taskID); //init
extern void EEGFunc_SetEegSampling(bool start); // start/stop the eeg sampling
extern void EEGFunc_SetEegSending(bool send); // start/stop sending the eeg data packet
extern void EEGFunc_SendEegPacket(uint16 connHandle); // send the eeg packet

#endif