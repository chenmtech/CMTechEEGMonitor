/*
 * App_EEGFunc.h : EEG application Function Model header file
 * Written by Chenm
 */

#ifndef APP_EEGFUNC_H
#define APP_EEGFUNC_H

#include "hal_types.h"

extern void EEGFunc_Init(uint8 taskID); //init
extern void EEGFunc_SetEegSampling(bool start); // is the eeg sampling started
extern void EEGFunc_SetEegSending(bool send); // is the eeg data sent?
extern void EEGFunc_SendEegPacket(uint16 connHandle); // send eeg packet

#endif