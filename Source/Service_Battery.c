
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "cmutil.h"

#include "Service_Battery.h"

#include "hal_adc.h"

/*
* 常量
*/
// 产生服务和特征的128位UUID
// 电池电量服务UUID
CONST uint8 batteryServUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(BATTERY_SERV_UUID)
};

// 电池电量数据UUID
CONST uint8 batteryDataUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(BATTERY_DATA_UUID)
};


/*
* 局部变量
*/
// 电池电量服务属性值
static CONST gattAttrType_t batteryService = { ATT_UUID_SIZE, batteryServUUID };

// 电池电量数据的相关属性：可读
static uint8 batteryDataProps = GATT_PROP_READ;
static uint8 batteryData = 0;

// 服务的属性表
static gattAttribute_t batteryServAttrTbl[] = 
{
  // battery 服务
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&batteryService                  /* pValue */
  },

    // 电池电量数据特征声明
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &batteryDataProps 
    },

      // 电池电量数据特征值
      { 
        { ATT_UUID_SIZE, batteryDataUUID },
        GATT_PERMIT_READ, 
        0, 
        &batteryData 
      },
      
};




/*
* 局部函数
*/

// 服务给协议栈的回调函数
// 读属性回调
static uint8 battery_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
// 写属性回调
static bStatus_t battery_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );
// 连接状态改变回调
static void battery_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

static uint8 battery_Measure(); 


// 服务给协议栈的回调结构体实例
CONST gattServiceCBs_t batteryServCBs =
{
  battery_ReadAttrCB,      // Read callback function pointer
  battery_WriteAttrCB,     // Write callback function pointer
  NULL                       // Authorization callback function pointer
};


static uint8 battery_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;
  uint16 uuid;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
  
  if (utilExtractUuid16(pAttr, &uuid) == FAILURE) {
    // Invalid handle
    *pLen = 0;
    return ATT_ERR_INVALID_HANDLE;
  }
  
  switch ( uuid )
  {
    // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
    // gattserverapp handles those reads
    case BATTERY_DATA_UUID:
      // 读电池电量值
      *pLen = 1;
      batteryData = battery_Measure();
      pValue[0] = *pAttr->pValue;
      break;
      
    default:
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }  
  
  return status;
}


static bStatus_t battery_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;
  uint16 uuid;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  if (utilExtractUuid16(pAttr,&uuid) == FAILURE) {
    // Invalid handle
    return ATT_ERR_INVALID_HANDLE;
  }
  
  switch ( uuid )
  {
    default:
      // Should never get here!
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }
  
  return status;
}

static void battery_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      //GATTServApp_InitCharCfg( connHandle, batteryDataConfig );
    }
  }
}

/*
 * 公共函数
*/

// 加载服务
extern bStatus_t Battery_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  //GATTServApp_InitCharCfg( INVALID_CONNHANDLE, batteryDataConfig );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( battery_HandleConnStatusCB );  
  
  if ( services & BATTERY_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( batteryServAttrTbl, 
                                          GATT_NUM_ATTRS( batteryServAttrTbl ),
                                          &batteryServCBs );
  }

  return ( status );
}

// 登记应用层给的回调
extern bStatus_t Battery_RegisterAppCBs( batteryServiceCBs_t *appCallbacks )
{
  return ( SUCCESS );
}

// 设置本服务的特征参数
extern bStatus_t Battery_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    // 设置电池电量数据，触发Notification
    case BATTERY_DATA:
      if ( len == sizeof(uint8) )
      {
        batteryData = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }  
  
  return ( ret );
}

// 获取本服务特征参数
extern bStatus_t Battery_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  
  switch ( param )
  {
    // 获取电池电量数据
    case BATTERY_DATA:
      *((uint8*)value) = batteryData;
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

// 测量电池电压
// P0_7为控制端，低电平启动，高电平截止
// P0_6为ADC测量端
static uint8 battery_Measure()
{
  
  P0 &= ~(1<<7);
  
  HalAdcInit();
  uint16 result = HalAdcRead (HAL_ADC_CHANNEL_6, HAL_ADC_RESOLUTION_14);
  
  P0 |= (1<<7);
  
  return (uint8)(result>>6);
  //return (uint8)((result>>6)*33/128);
}
