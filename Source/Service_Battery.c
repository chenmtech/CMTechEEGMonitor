
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
* ����
*/
// ���������������128λUUID
// ��ص�������UUID
CONST uint8 batteryServUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(BATTERY_SERV_UUID)
};

// ��ص�������UUID
CONST uint8 batteryDataUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(BATTERY_DATA_UUID)
};


/*
* �ֲ�����
*/
// ��ص�����������ֵ
static CONST gattAttrType_t batteryService = { ATT_UUID_SIZE, batteryServUUID };

// ��ص������ݵ�������ԣ��ɶ�
static uint8 batteryDataProps = GATT_PROP_READ;
static uint8 batteryData = 0;

// ��������Ա�
static gattAttribute_t batteryServAttrTbl[] = 
{
  // battery ����
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&batteryService                  /* pValue */
  },

    // ��ص���������������
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &batteryDataProps 
    },

      // ��ص�����������ֵ
      { 
        { ATT_UUID_SIZE, batteryDataUUID },
        GATT_PERMIT_READ, 
        0, 
        &batteryData 
      },
      
};




/*
* �ֲ�����
*/

// �����Э��ջ�Ļص�����
// �����Իص�
static uint8 battery_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
// д���Իص�
static bStatus_t battery_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );
// ����״̬�ı�ص�
static void battery_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

static uint8 battery_Measure(); 


// �����Э��ջ�Ļص��ṹ��ʵ��
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
      // ����ص���ֵ
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
 * ��������
*/

// ���ط���
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

// �Ǽ�Ӧ�ò���Ļص�
extern bStatus_t Battery_RegisterAppCBs( batteryServiceCBs_t *appCallbacks )
{
  return ( SUCCESS );
}

// ���ñ��������������
extern bStatus_t Battery_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    // ���õ�ص������ݣ�����Notification
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

// ��ȡ��������������
extern bStatus_t Battery_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  
  switch ( param )
  {
    // ��ȡ��ص�������
    case BATTERY_DATA:
      *((uint8*)value) = batteryData;
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

// ������ص�ѹ
// P0_7Ϊ���ƶˣ��͵�ƽ�������ߵ�ƽ��ֹ
// P0_6ΪADC������
static uint8 battery_Measure()
{
  
  P0 &= ~(1<<7);
  
  HalAdcInit();
  uint16 result = HalAdcRead (HAL_ADC_CHANNEL_6, HAL_ADC_RESOLUTION_14);
  
  P0 |= (1<<7);
  
  return (uint8)(result>>6);
  //return (uint8)((result>>6)*33/128);
}
