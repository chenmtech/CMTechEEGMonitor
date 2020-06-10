/*
* Dev_Battery: �����ѹ��ͨ��ADC������ص���
* Written by Chenm 2019-02-24
*/


#include "Dev_Battery.h"

#define BATTERY_ADC_CHANNEL HAL_ADC_CHANNEL_6

// ������ص����ٷֱ�
extern uint8 Battery_Measure()
{
  P0 &= ~(1<<7);
  
  HalAdcInit();
  uint16 result = HalAdcRead (BATTERY_ADC_CHANNEL, HAL_ADC_RESOLUTION_14);
  
  P0 |= (0x01<<7);
  
  //return (uint8)(result>>6);
  return (uint8)((result>>6)*33/128);
}