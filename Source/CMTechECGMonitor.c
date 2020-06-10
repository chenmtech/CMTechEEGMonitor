/**************************************************************************************************
  Filename:       CMTechECGMonitor.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED �AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"


#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "Service_ECGMonitor.h"

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif


#include "App_GAPConfig.h"

#include "App_GATTConfig.h"

#include "App_ECGFunc.h"

#include "Dev_ADS1x9x.h"

#include "CMTechECGMonitor.h"

#include "Service_Battery.h"


/*********************************************************************
 * ����
 */


#define INVALID_CONNHANDLE                    0xFFFF


// ֹͣ�ɼ�״̬
#define STATUS_STOP           0     

// ��ʼ�ɼ�״̬
#define STATUS_START          1    


/*********************************************************************
 * �ֲ�����
 */

// ����ID
static uint8 ecgMonitor_TaskID;  

// GAP״̬
static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP GATT �豸��
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "ECGMonitor";


/*********************************************************************
 * �ֲ�����
 */
// OSAL��Ϣ������
static void ecgMonitor_ProcessOSALMsg( osal_event_hdr_t *pMsg );

// ״̬�仯֪ͨ�ص�����
static void peripheralStateNotificationCB( gaprole_States_t newState );

// ECGMonitor����ص�����
static void ecgMonitorServiceCB( uint8 paramID );

// ��ص�������ص�����
static void batteryServiceCB( uint8 paramID );

// ��ʼ��IO�ܽ�
static void ecgMonitorInitIOPin();


/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t ecgMonitor_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t ecgMonitor_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// ��ʪ�Ȼص��ṹ��ʵ�����ṹ����Serice_TempHumid��������
static ecgServiceCBs_t ecgMonitor_ServCBs =
{
  ecgMonitorServiceCB    // ��ʪ�ȷ���ص�����ʵ����������Serice_TempHumid��������
};

// ��ص�������ص��ṹ��ʵ�����ṹ����Serice_Battery��������
static batteryServiceCBs_t battery_ServCBs =
{
  batteryServiceCB    // ��ص�������ص�����ʵ����������Serice_Battery��������
};



/*********************************************************************
 * ��������
 */

extern void ECGMonitor_Init( uint8 task_id )
{
  ecgMonitor_TaskID = task_id;

  // GAP ����
  //���ù㲥����
  GAPConfig_SetAdvParam(2000, ECGMONITOR_SERV_UUID);
  
  // ��ʼ�����̹㲥
  GAPConfig_EnableAdv(TRUE);

  //�������Ӳ���
  GAPConfig_SetConnParam(20, 40, 0, 1500, 1);

  //����GGS�������豸��
  GAPConfig_SetGGSParam(attDeviceName);

  //���ð󶨲���
  GAPConfig_SetBondingParam(0, GAPBOND_PAIRING_MODE_NO_PAIRING);

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  ECGMonitor_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif
  
  GATTConfig_SetECGMonitorService(&ecgMonitor_ServCBs);
  
  GATTConfig_SetBatteryService(&battery_ServCBs);
  

  //�������ʼ��GPIO
  //��һ�����йܽţ�reset���״̬�������������
  //�ڶ������ڲ��õ�IO�����鲻���ӵ��ⲿ��·������Ϊ��������
  //���������ڻ��õ���IO����Ҫ���ݾ����ⲿ��·�������������Ч���ã���ֹ�ĵ�
  {
    ecgMonitorInitIOPin();
  }
  
  // ��ʼ��
  ECGFunc_Init(); 

  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

  // ���������豸�¼�
  osal_set_event( ecgMonitor_TaskID, ECGMONITOR_START_DEVICE_EVT );

}

// ��ʼ��IO�ܽ�
static void ecgMonitorInitIOPin()
{
  // ȫ����ΪGPIO
  P0SEL = 0; 
  P1SEL = 0; 
  P2SEL = 0; 

  // ȫ����Ϊ����͵�ƽ
  P0DIR = 0xFF; 
  P1DIR = 0xFF; 
  P2DIR = 0x1F; 

  P0 = 0; 
  P1 = 0;   
  P2 = 0; 
  
  // ��ص�ѹ����������
  // P0_7Ϊ���ƶˣ��͵�ƽ�������ߵ�ƽ��ֹ��
  // ��������Ϊ����ߵ�ƽ
  P0DIR |= (1<<7);
  P0 |= (1<<7); 
  
  // P0_6Ϊ��ص�ѹ��ADC�����ˣ�����Ϊ����
  P0DIR &= ~(1<<6);
  
  // I2C��SDA, SCL����ΪGPIO, ����͵�ƽ�����򹦺ĺܴ�
  //HalI2CSetAsGPIO();
}

extern uint16 ECGMonitor_ProcessEvent( uint8 task_id, uint16 events )
{

  VOID task_id; // OSAL required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( ecgMonitor_TaskID )) != NULL )
    {
      ecgMonitor_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );

      // Release the OSAL message
      VOID osal_msg_deallocate( pMsg );
    }

    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & ECGMONITOR_START_DEVICE_EVT )
  {    
    // Start the Device
    VOID GAPRole_StartDevice( &ecgMonitor_PeripheralCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &ecgMonitor_BondMgrCBs );

    return ( events ^ ECGMONITOR_START_DEVICE_EVT );
  }

  // Discard unknown events
  return 0;
}

static void ecgMonitor_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
  switch ( pMsg->event )
  {

  default:
    // do nothing
    break;
  }
}


/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);
      }
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          
        //ECGFunc_Init();    
      }
      break;

    case GAPROLE_CONNECTED:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          
        //GAPRole_GetParameter( GAPROLE_CONNHANDLE, &gapConnHandle );  
        //ECGFunc_Init();   
      }
      break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
        /*  
        enable = false;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &enable );  
        // ���ӶϿ���ֹͣ����  
        ECGFunc_Stop();
        ECGFunc_Init();
        enable = true;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &enable );
        */  
        while(1) {
          HAL_SYSTEM_RESET();  
        }
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
        /*  
        enable = false;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &enable );  
        // ���ӶϿ���ֹͣ����  
        ECGFunc_Stop();
        ECGFunc_Init();
        enable = true;
        GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &enable );
        */  
        while(1) {
          HAL_SYSTEM_RESET();  
        }
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          
        //GAPConfig_TerminateConn();
          
        // ���ӶϿ���ֹͣ����  
        //ECGFunc_Stop();
        //ADS1x9x_Reset();
        while(1) {
          HAL_SYSTEM_RESET();  
        }  
      }
      break;

    default:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          
        //GAPConfig_TerminateConn();
          
        // ���ӶϿ���ֹͣ����  
        //ECGFunc_Stop();
        //ADS1x9x_Reset();   
        while(1) {
          HAL_SYSTEM_RESET();  
        }  
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}


// ECGMonitor����ص�����
static void ecgMonitorServiceCB( uint8 paramID )
{
  uint8 newValue;

  switch (paramID)
  {
    case ECGMONITOR_CTRL:
      ECGMonitor_GetParameter( ECGMONITOR_CTRL, &newValue );
      
      // ֹͣ�ɼ�
      if ( newValue == ECGMONITOR_CTRL_STOP)  
      {
        ECGFunc_Stop();
      }
      // ��ʼ�ɼ�ECG
      else if ( newValue == ECGMONITOR_CTRL_START_ECG) 
      {
        //ADS1x9x_Reset();
        ECGFunc_StartEcg();
      }
      // ��ʼ�ɼ�1mV
      else if ( newValue == ECGMONITOR_CTRL_START_1MV) 
      {
        //ADS1x9x_Reset();
        ECGFunc_Start1mV();
      }
      
      break;

    default:
      // Should not get here
      break;
  }
}

static void batteryServiceCB( uint8 paramID )
{
  
}


//extern void CMTechECGMonitor_SendECGSignals(uint8* pData, uint8 len)
//{
//  uint8* p = ECGSignalAtt.value;
//  
//  VOID osal_memcpy( p, pData, len );
//  
//  ECGSignalAtt.len = len;
//  ECGSignalAtt.handle = 0;  
//  
//  //if(gapConnHandle != NULL)
//    ECGMonitor_ECGSignalNotify( gapConnHandle, &ECGSignalAtt);
//  
//}

/*********************************************************************
*********************************************************************/
