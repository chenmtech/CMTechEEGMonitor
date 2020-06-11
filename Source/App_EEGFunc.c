/*
 * App_EEGFunc.h : EEG application Function Model source file
 * Written by Chenm
 */

#include "App_EEGFunc.h"
#include "CMUtil.h"
#include "Dev_ADS1x9x.h"
#include "service_EEG.h"
#include "CMTechEEGMonitor.h"


#define EEG_PACK_BYTE_NUM 19 // byte number per eeg packet, 1+6*3
#define EEG_MAX_PACK_NUM 255 // max packet num

static uint8 taskId; // taskId of application

// is the eeg data sent?
static bool eegSend = false;
// the number of the current eeg data packet, from 0 to EEG_MAX_PACK_NUM
static uint8 pckNum = 0;
// eeg packet buffer
static uint8 eegBuff[EEG_PACK_BYTE_NUM] = {0};
// pointer to the eeg buff
static uint8* pEegBuff;
// eeg packet structure sent out
static attHandleValueNoti_t eegNoti;

static void processEegSignal(uint8 hByte, uint8 mByte, uint8 lByte);
static void saveEegSignal(uint8 hByte, uint8 mByte, uint8 lByte);
//static void processTestSignal(int16 x);

extern void EEGFunc_Init(uint8 taskID)
{ 
  taskId = taskID;
  
  // initilize the ADS1x9x and set the data process callback function
  ADS1x9x_Init(processEegSignal); 
  
  delayus(1000);
}

extern void EEGFunc_SetEegSampling(bool start)
{
  if(start)
  {
    ADS1x9x_WakeUp(); 
    // 这里一定要延时，否则容易死机
    delayus(1000);
    ADS1x9x_StartConvert();
    delayus(1000);
  } 
  else
  {
    ADS1x9x_StopConvert();
    ADS1x9x_StandBy();
    delayus(2000);
  }
}

extern void EEGFunc_SetEegSending(bool send)
{
  if(send)
  {
    pckNum = 0;
    pEegBuff = eegBuff;
    osal_clear_event(taskId, EEG_PACK_NOTI_EVT);
  }
  eegSend = send;
}

extern void EEGFunc_SendEegPacket(uint16 connHandle)
{
  EEG_PacketNotify( connHandle, &eegNoti );
}

static void processEegSignal(uint8 hByte, uint8 mByte, uint8 lByte)
{
  if(eegSend) // need send ecg
  {
    saveEegSignal(hByte, mByte, lByte);
  }
}

static void saveEegSignal(uint8 hByte, uint8 mByte, uint8 lByte)
{
  if(pEegBuff == eegBuff)
  {
    *pEegBuff++ = pckNum;
    if(pckNum == EEG_MAX_PACK_NUM)
      pckNum = 0;
    else
      pckNum++;
  }
  *pEegBuff++ = lByte;  
  *pEegBuff++ = mByte;
  *pEegBuff++ = hByte;
  
  if(pEegBuff-eegBuff >= EEG_PACK_BYTE_NUM)
  {
    osal_memcpy(eegNoti.value, eegBuff, EEG_PACK_BYTE_NUM);
    eegNoti.len = EEG_PACK_BYTE_NUM;
    osal_set_event(taskId, EEG_PACK_NOTI_EVT);
    pEegBuff = eegBuff;
  }
}

//static void processTestSignal(int16 x)
//{
//  static int16 data1mV[125] = {0};
//  static uint8 index = 0;
//  uint8 i,j;
//  
//  data1mV[index++] = x;
//  
//  if(index >= SAMPLERATE)
//  {
//    uint16 tmp; 
//    for(i = 0; i < SAMPLERATE; ++i)
//    {
//      for(j = i+1; j < SAMPLERATE; j++)
//      {
//        if(data1mV[j] < data1mV[i])
//        {
//          tmp = data1mV[i];
//          data1mV[i] = data1mV[j];
//          data1mV[j] = tmp;
//        }
//      }
//    }
//    long smin = 0;
//    long smax = 0;
//    for(i = 25; i < 35; i++)
//    {
//      smin += data1mV[i];
//    }
//    for(i = 90; i < 100; i++)
//    {
//      smax += data1mV[i];
//    }
//    caliValue = (smax-smin)/20;
//    index = 0;
//  }
//}