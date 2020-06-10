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
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
 * 常量
 */


#define INVALID_CONNHANDLE                    0xFFFF


// 停止采集状态
#define STATUS_STOP           0     

// 开始采集状态
#define STATUS_START          1    


/*********************************************************************
 * 局部变量
 */

// 任务ID
static uint8 ecgMonitor_TaskID;  

// GAP状态
static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP GATT 设备名
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "ECGMonitor";


/*********************************************************************
 * 局部函数
 */
// OSAL消息处理函数
static void ecgMonitor_ProcessOSALMsg( osal_event_hdr_t *pMsg );

// 状态变化通知回调函数
static void peripheralStateNotificationCB( gaprole_States_t newState );

// ECGMonitor服务回调函数
static void ecgMonitorServiceCB( uint8 paramID );

// 电池电量服务回调函数
static void batteryServiceCB( uint8 paramID );

// 初始化IO管脚
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

// 温湿度回调结构体实例，结构体是Serice_TempHumid中声明的
static ecgServiceCBs_t ecgMonitor_ServCBs =
{
  ecgMonitorServiceCB    // 温湿度服务回调函数实例，函数是Serice_TempHumid中声明的
};

// 电池电量服务回调结构体实例，结构体是Serice_Battery中声明的
static batteryServiceCBs_t battery_ServCBs =
{
  batteryServiceCB    // 电池电量服务回调函数实例，函数是Serice_Battery中声明的
};



/*********************************************************************
 * 公共函数
 */

extern void ECGMonitor_Init( uint8 task_id )
{
  ecgMonitor_TaskID = task_id;

  // GAP 配置
  //配置广播参数
  GAPConfig_SetAdvParam(2000, ECGMONITOR_SERV_UUID);
  
  // 初始化立刻广播
  GAPConfig_EnableAdv(TRUE);

  //配置连接参数
  GAPConfig_SetConnParam(20, 40, 0, 1500, 1);

  //配置GGS，设置设备名
  GAPConfig_SetGGSParam(attDeviceName);

  //配置绑定参数
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
  

  //在这里初始化GPIO
  //第一：所有管脚，reset后的状态都是输入加上拉
  //第二：对于不用的IO，建议不连接到外部电路，且设为输入上拉
  //第三：对于会用到的IO，就要根据具体外部电路连接情况进行有效设置，防止耗电
  {
    ecgMonitorInitIOPin();
  }
  
  // 初始化
  ECGFunc_Init(); 

  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

  // 发送启动设备事件
  osal_set_event( ecgMonitor_TaskID, ECGMONITOR_START_DEVICE_EVT );

}

// 初始化IO管脚
static void ecgMonitorInitIOPin()
{
  // 全部设为GPIO
  P0SEL = 0; 
  P1SEL = 0; 
  P2SEL = 0; 

  // 全部设为输出低电平
  P0DIR = 0xFF; 
  P1DIR = 0xFF; 
  P2DIR = 0x1F; 

  P0 = 0; 
  P1 = 0;   
  P2 = 0; 
  
  // 电池电压测量的设置
  // P0_7为控制端，低电平启动，高电平截止。
  // 这里设置为输出高电平
  P0DIR |= (1<<7);
  P0 |= (1<<7); 
  
  // P0_6为电池电压的ADC测量端，设置为输入
  P0DIR &= ~(1<<6);
  
  // I2C的SDA, SCL设置为GPIO, 输出低电平，否则功耗很大
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
        // 连接断开后停止采样  
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
        // 连接断开后停止采样  
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
          
        // 连接断开后停止采样  
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
          
        // 连接断开后停止采样  
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


// ECGMonitor服务回调函数
static void ecgMonitorServiceCB( uint8 paramID )
{
  uint8 newValue;

  switch (paramID)
  {
    case ECGMONITOR_CTRL:
      ECGMonitor_GetParameter( ECGMONITOR_CTRL, &newValue );
      
      // 停止采集
      if ( newValue == ECGMONITOR_CTRL_STOP)  
      {
        ECGFunc_Stop();
      }
      // 开始采集ECG
      else if ( newValue == ECGMONITOR_CTRL_START_ECG) 
      {
        //ADS1x9x_Reset();
        ECGFunc_StartEcg();
      }
      // 开始采集1mV
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
