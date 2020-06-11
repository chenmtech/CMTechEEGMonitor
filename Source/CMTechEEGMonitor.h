/**************************************************************************************************
* CMTechEEGMonitor.h : EEG Monitor application header file
**************************************************************************************************/

#ifndef CMTECHEEG_H
#define CMTECHEEG_H


#define EEG_START_DEVICE_EVT 0x0001      // device start event
#define EEG_PACK_NOTI_EVT 0x0002         // eeg packet notification event
#define EEG_BATT_PERIODIC_EVT 0x0004     // periodic battery measurement event

/*
 * Task Initialization for the BLE Application
 */
extern void EEG_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 EEG_ProcessEvent( uint8 task_id, uint16 events );


#endif 
