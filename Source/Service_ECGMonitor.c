

/*********************************************************************
 * INCLUDES
 */
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"
#include "cmutil.h"

#include "dev_ads1x9x.h"
#include "Service_ECGMonitor.h"
#include "CMTechECGMonitor.h"


/*********************************************************************
 * ����
 */

// ECGMonitor����UUID
CONST uint8 ecgServUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(ECGMONITOR_SERV_UUID)
};

// ECG��������UUID
CONST uint8 ecgDataUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(ECGMONITOR_DATA_UUID)
};

// �������Ƶ�����UUID
CONST uint8 ecgCtrlUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(ECGMONITOR_CTRL_UUID)
};

// 1mV�ź�����UUID
CONST uint8 ecg1mVUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(ECGMONITOR_1MV_UUID)
};

// ����������UUID
CONST uint8 ecgSampleRateUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(ECGMONITOR_SAMPLERATE_UUID)
};

// ������������UUID
CONST uint8 ecgLeadTypeUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(ECGMONITOR_LEADTYPE_UUID)
};





/*********************************************************************
 * �ֲ�����
 */



/*********************************************************************
 * Profile Attributes - variables
 */

// ECGMonitor�ķ���Attribute����
static CONST gattAttrType_t ecgService = { ATT_UUID_SIZE, ecgServUUID };

// ECG��������
static uint8 ecgDataProps = GATT_PROP_READ | GATT_PROP_NOTIFY;
static uint8 ecgData[ECG_PACKET_LEN] = {0};
static gattCharCfg_t ecgDataConfig[GATT_MAX_NUM_CONN];  //ÿ�������϶�Ӧ��һ������ֵ

// �������Ƶ�����
static uint8 ecgCtrlProps = GATT_PROP_READ | GATT_PROP_WRITE;
static uint8 ecgCtrl  = ECGMONITOR_CTRL_STOP;

//1mV�ź�����
static uint8 ecg1mVProps = GATT_PROP_READ;
static int ecg1mV = 0;

// ����������
static uint8 ecgSampleRateProps = GATT_PROP_READ;
static int ecgSampleRate = SR_SPS;      //������

// Lead Type Characteristic
static uint8 ecgLeadTypeProps = GATT_PROP_READ;
static uint8 ecgLeadType  = LEADTYPE_I;  //Lead I



/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t ecgMonitorServAttrTbl[] = 
{
  // 0. ECG Monitor Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&ecgService                      /* pValue */
  },
      
    // ECG����
    // Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &ecgDataProps 
    },

      // Characteristic Value
      { 
        { ATT_UUID_SIZE, ecgDataUUID },
        0, 
        0, 
        (uint8 *)ecgData 
      },
  
      // Characteristic Configuration 
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)ecgDataConfig
      },
      
    // �������Ƶ�����
    // Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &ecgCtrlProps 
    },

      // Characteristic Value
      { 
        { ATT_UUID_SIZE, ecgCtrlUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0, 
        &ecgCtrl 
      },  
 
    // 1mV�ź�����   
    // Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &ecg1mVProps 
    },

      // Characteristic Value
      { 
        { ATT_UUID_SIZE, ecg1mVUUID },
        GATT_PERMIT_READ, 
        0, 
        (uint8*)&ecg1mV 
      },
    
    // ����������
    // Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &ecgSampleRateProps 
    },

      // Characteristic Value
      { 
        { ATT_UUID_SIZE, ecgSampleRateUUID },
        GATT_PERMIT_READ, 
        0, 
        (uint8*)&ecgSampleRate 
      },    
    
    // ������������   
    // Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &ecgLeadTypeProps 
    },

      // Characteristic Value
      { 
        { ATT_UUID_SIZE, ecgLeadTypeUUID },
        GATT_PERMIT_READ, 
        0, 
        &ecgLeadType 
      },        

};

//��������Ӧ�ò�Ļص��ṹ��ʵ��ָ��
static ecgServiceCBs_t * ecgService_AppCBs = NULL;


/*********************************************************************
 * �ֲ�����
 */

static uint8 ecgMonitor_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t ecgMonitor_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );

static void ecgMonitor_HandleConnStatusCB( uint16 connHandle, uint8 changeType );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// ����Service��GATT�Ļص�����Ҫ��Ӧ�ò�Ļص������
CONST gattServiceCBs_t ecgMonitorServCBs =
{
  ecgMonitor_ReadAttrCB,  // Read callback function pointer
  ecgMonitor_WriteAttrCB, // Write callback function pointer
  NULL                     // Authorization callback function pointer
};



// ��pAttr�ж�һ��Attribute��ֵ�ŵ�pValue��ȥ
static uint8 ecgMonitor_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
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
    case ECGMONITOR_DATA_UUID:
      // ��ECG���ݰ�ֵ
      *pLen = ECG_PACKET_LEN;
      VOID osal_memcpy( pValue, pAttr->pValue, ECG_PACKET_LEN );
      break;
      
    case ECGMONITOR_CTRL_UUID:
    case ECGMONITOR_LEADTYPE_UUID:
      *pLen = 1;
      pValue[0] = *pAttr->pValue;
      break;
    
    case ECGMONITOR_1MV_UUID:
    case ECGMONITOR_SAMPLERATE_UUID:
      *pLen = sizeof(int);
      VOID osal_memcpy( pValue, pAttr->pValue, sizeof(int) );
      break;
      
    default:
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }  
  
  return status;
}

// дһ��Attribute����pValueд��pAttr��ȥ
static bStatus_t ecgMonitor_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                          uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
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
    case ECGMONITOR_DATA_UUID:
    case ECGMONITOR_1MV_UUID:
    case ECGMONITOR_SAMPLERATE_UUID:
    case ECGMONITOR_LEADTYPE_UUID:
      // ����д
      break;

    // д�������Ƶ�
    case ECGMONITOR_CTRL_UUID:
      // Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 )
      {
        if ( len != 1 )
        {
          status = ATT_ERR_INVALID_VALUE_SIZE;
        }
      }
      else
      {
        status = ATT_ERR_ATTR_NOT_LONG;
      }

      // Write the value
      if ( status == SUCCESS )
      {
        uint8 *pCurValue = (uint8 *)pAttr->pValue;

        *pCurValue = pValue[0];

        if( pAttr->pValue == &ecgCtrl )
        {
          notifyApp = ECGMONITOR_CTRL;
        }
      }
      break;

    // дECG���ݵ�CCC  
    case GATT_CLIENT_CHAR_CFG_UUID:
      status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                              offset, GATT_CLIENT_CFG_NOTIFY );
      break;

    default:
      // Should never get here!
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && ecgService_AppCBs && ecgService_AppCBs->pfnEcgServiceCB )
  {
    ecgService_AppCBs->pfnEcgServiceCB( notifyApp );
  }  
  
  return status;
}

// ����״̬�ı�ص�����
static void ecgMonitor_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      GATTServApp_InitCharCfg( connHandle, ecgDataConfig );
    }
  }
}


/*********************************************************************
 * PUBLIC FUNCTIONS
 */
extern bStatus_t ECGMonitor_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, ecgDataConfig );

  // Register with Link DB to receive link status change callback
  VOID linkDB_Register( ecgMonitor_HandleConnStatusCB );  
  
  if ( services & ECGMONITOR_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( ecgMonitorServAttrTbl, 
                                          GATT_NUM_ATTRS( ecgMonitorServAttrTbl ),
                                          &ecgMonitorServCBs );
  }

  return ( status );
}

//�Ǽ�Ӧ�ò�ص��ṹ��ʵ��ָ��
extern bStatus_t ECGMonitor_RegisterAppCBs( ecgServiceCBs_t * appCallbacks )
{
  if ( appCallbacks )
  {
    ecgService_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

// ��Ӧ�ò���ã����÷�������ֵ
extern bStatus_t ECGMonitor_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;

  switch ( param )
  {
    // ����ECG���ݣ�����Notification
    case ECGMONITOR_DATA:
      if ( len == ECG_PACKET_LEN )
      {
        // ����ecgData�ǹ����ⲿ���ݴ���App_ECGFuncģ�飬����value��Ȼ����ecgData
        // ������ȥ�˿�������
        //VOID osal_memcpy( ecgData, value, ECG_PACKET_LEN );
        if(value != ecgData) {
          ret = bleInvalidRange;
          break;
        }
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( ecgDataConfig, ecgData, FALSE,
                                   ecgMonitorServAttrTbl, GATT_NUM_ATTRS( ecgMonitorServAttrTbl ),
                                   INVALID_TASK_ID );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    // ���ò������Ƶ�
    case ECGMONITOR_CTRL:
      if ( len == sizeof ( uint8 ) )
      {
        ecgCtrl = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
      
    // ����1mV����ֵ
    case ECGMONITOR_1MV:
      if ( len == sizeof ( int ) )
      {
        VOID osal_memcpy( (uint8*)&ecg1mV, value, sizeof(int) );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
      
    // ���ò�����
    case ECGMONITOR_SAMPLERATE:
      if ( len == sizeof ( int ) )
      {
        VOID osal_memcpy( (uint8*)&ecgSampleRate, value, sizeof(int) );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;  
      
      
    // ���õ�������  
    case ECGMONITOR_LEADTYPE:
      if ( len == sizeof ( uint8 ) )
      {
        ecgLeadType = *((uint8*)value);
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

// ��Ӧ�ò���ã���ȡ��������ֵ
extern bStatus_t ECGMonitor_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  
  switch ( param )
  {
    // ��ȡECG����
    case ECGMONITOR_DATA:
      VOID osal_memcpy( value, ecgData, ECG_PACKET_LEN );
      break;

    // ��ȡ�������Ƶ�  
    case ECGMONITOR_CTRL:
      *((uint8*)value) = ecgCtrl;
      break;

    // ��ȡ1mV�ź�  
    case ECGMONITOR_1MV:
      VOID osal_memcpy( value, (uint8*)&ecg1mV, sizeof(int) );
      break;
      
    // ��ȡ������ 
    case ECGMONITOR_SAMPLERATE:
      VOID osal_memcpy( value, (uint8*)&ecgSampleRate, sizeof(int) );
      break;      
      
    // ��ȡ��������  
    case ECGMONITOR_LEADTYPE:
      *((uint8*)value) = ecgLeadType;
      break;      

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

// ����ecgData���ⲿ���ݴ���ģ�飬��ȥ�ظ��������ݿռ�
extern uint8 * ECGMonitor_GetECGDataPointer()
{
  return ecgData;
}

//����һ��ECG�ź�ֵ��Notification
//extern bStatus_t CMTechECGMonitor_ECGSignalNotify( uint16 connHandle, attHandleValueNoti_t *pNoti )
//{
//  uint16 value = GATTServApp_ReadCharCfg( connHandle, ECGSignalConfig );
//
//  // If notifications enabled
//  if ( value & GATT_CLIENT_CFG_NOTIFY )
//  {
//    // Set the handle
//    pNoti->handle = ECGMonitorAttrTbl[ECGSIGNAL_VALUE_POS].handle;
//  
//    // Send the Notification
//    return GATT_Notification( connHandle, pNoti, FALSE );
//  }
//
//  return bleIncorrectMode;
//}




/*********************************************************************
*********************************************************************/
