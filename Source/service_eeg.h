/**
* ecg service header file: providing the ecg-related info and sending the ecg data packet
*/

#ifndef SERVICE_EEG_H
#define SERVICE_EEG_H

// EEG Service Parameters
#define EEG_PACK                      0  // eeg data packet
#define EEG_PACK_CHAR_CFG             1  // 
#define EEG_1MV_CALI                  2  // 1mV calibration value
#define EEG_SAMPLE_RATE               3  // sample rate
#define EEG_LEAD_TYPE                 4  // lead type

// EEG Service UUIDs
#define EEG_SERV_UUID                 0xAA40
#define EEG_PACK_UUID                 0xAA41
#define EEG_1MV_CALI_UUID             0xAA42
#define EEG_SAMPLE_RATE_UUID          0xAA43
#define EEG_LEAD_TYPE_UUID            0xAA44

// EEG Service bit fields
#define EEG_SERVICE                   0x00000001

// Callback events
#define EEG_PACK_NOTI_ENABLED         0 // eeg data packet notification enabled
#define EEG_PACK_NOTI_DISABLED        1 // eeg data packet notification disabled

// eeg Service callback function
typedef void (*eegServiceCB_t)(uint8 event);

typedef struct
{
  eegServiceCB_t    pfnEegServiceCB;  
} EEGServiceCBs_t;


extern bStatus_t EEG_AddService( uint32 services );
extern void EEG_RegisterAppCBs( EEGServiceCBs_t* pfnServiceCBs );
extern bStatus_t EEG_SetParameter( uint8 param, uint8 len, void *value );
extern bStatus_t EEG_GetParameter( uint8 param, void *value );
extern bStatus_t EEG_PacketNotify( uint16 connHandle, attHandleValueNoti_t *pNoti );// notify the eeg data packet



#endif /* ECGSERVICE_H */
