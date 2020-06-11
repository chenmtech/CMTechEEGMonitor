/**
* eeg service source file: providing the eeg-related info and sending the eeg data packet
*/

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "CMUtil.h"
#include "Service_EEG.h"

// Position of EEG data packet in attribute array
#define EEG_PACK_VALUE_POS            2

#define DEFAULT_SAMPLERATE 250

// EEG service
CONST uint8 EEGServUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(EEG_SERV_UUID)
};

// EEG Data Packet characteristic
CONST uint8 EEGPackUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(EEG_PACK_UUID)
};

// 1mV calibration characteristic
CONST uint8 EEG1mVCaliUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(EEG_1MV_CALI_UUID)
};

// Sample rate characteristic
CONST uint8 EEGSampleRateUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(EEG_SAMPLE_RATE_UUID)
};

// Lead Type characteristic
CONST uint8 EEGLeadTypeUUID[ATT_UUID_SIZE] =
{ 
  CM_UUID(EEG_LEAD_TYPE_UUID)
};

static EEGServiceCBs_t* eegServiceCBs;

// Ecg Service attribute
static CONST gattAttrType_t eegService = { ATT_UUID_SIZE, EEGServUUID };

// EEG Data Packet Characteristic
// Note: the characteristic value is not stored here
static uint8 eegPackProps = GATT_PROP_NOTIFY;
static uint8 eegPack = 0;
static gattCharCfg_t eegPackClientCharCfg[GATT_MAX_NUM_CONN];

// 1mV Calibration Characteristic
static uint8 eeg1mVCaliProps = GATT_PROP_READ;
static uint16 eeg1mVCali = 0;

// Sample Rate Characteristic
static uint8 eegSampleRateProps = GATT_PROP_READ;
static uint16 eegSampleRate = DEFAULT_SAMPLERATE;

// Lead Type Characteristic
static uint8 eegLeadTypeProps = GATT_PROP_READ;
static uint8 eegLeadType = LEADTYPE_I;


/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t EEGAttrTbl[] = 
{
  // EEG Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&eegService                      /* pValue */
  },

    // 1. EEG Data Packet Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &eegPackProps 
    },

      // EEG Data Packet Value
      { 
        { ATT_UUID_SIZE, EEGPackUUID },
        0, 
        0, 
        &eegPack 
      },

      // EEG Data Packet Client Characteristic Configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *) &eegPackClientCharCfg 
      },      

    // 2. 1mV Calibration Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &eeg1mVCaliProps 
    },

      // 1mV Calibration Value
      { 
        { ATT_UUID_SIZE, EEG1mVCaliUUID },
        GATT_PERMIT_READ, 
        0, 
        (uint8*)&eeg1mVCali 
      },

    // 3. Sample Rate Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &eegSampleRateProps 
    },

      // Sample Rate Value
      { 
        { ATT_UUID_SIZE, EEGSampleRateUUID },
        GATT_PERMIT_READ, 
        0, 
        (uint8*)&eegSampleRate 
      },

    // 4. Lead Type Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &eegLeadTypeProps 
    },

      // Lead Type Value
      { 
        { ATT_UUID_SIZE, EEGLeadTypeUUID },
        GATT_PERMIT_READ, 
        0, 
        &eegLeadType 
      } 
};

static uint8 readAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen );
static bStatus_t writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset );
static void handleConnStatusCB( uint16 connHandle, uint8 changeType );

// EEG Service Callbacks
CONST gattServiceCBs_t eegCBs =
{
  readAttrCB,  // Read callback function pointer
  writeAttrCB, // Write callback function pointer
  NULL         // Authorization callback function pointer
};

bStatus_t EEG_AddService( uint32 services )
{
  uint8 status = SUCCESS;

  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, eegPackClientCharCfg );
  
  VOID linkDB_Register(handleConnStatusCB);

  if ( services & EEG_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( EEGAttrTbl, 
                                          GATT_NUM_ATTRS( EEGAttrTbl ),
                                          &eegCBs );
  }

  return ( status );
}

extern void EEG_RegisterAppCBs( EEGServiceCBs_t* pfnServiceCBs )
{
  eegServiceCBs = pfnServiceCBs;
    
  return;
}

extern bStatus_t EEG_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
     case EEG_PACK_CHAR_CFG:
      // Need connection handle
      //ECGMeasClientCharCfg.value = *((uint16*)value);
      break;      

    case EEG_1MV_CALI:
      osal_memcpy((uint8*)&eeg1mVCali, value, len);
      break;
      
    case EEG_SAMPLE_RATE:
      osal_memcpy((uint8*)&eegSampleRate, value, len);
      break;
      
    case EEG_LEAD_TYPE:  
      eegLeadType = *((uint8*)value);
      break;

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

extern bStatus_t EEG_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case EEG_PACK_CHAR_CFG:
      // Need connection handle
      //*((uint16*)value) = ECGMeasClientCharCfg.value;
      break;      

    case EEG_1MV_CALI:
      osal_memcpy(value, (uint8*)&eeg1mVCali, 2);
      break;
      
    case EEG_SAMPLE_RATE:
      osal_memcpy(value, (uint8*)&eegSampleRate, 2);
      break;      
      
    case EEG_LEAD_TYPE:  
      *((uint8*)value) = eegLeadType;
      break; 

    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

extern bStatus_t EEG_PacketNotify( uint16 connHandle, attHandleValueNoti_t *pNoti )
{
  uint16 value = GATTServApp_ReadCharCfg( connHandle, eegPackClientCharCfg );

  // If notifications enabled
  if ( value & GATT_CLIENT_CFG_NOTIFY )
  {
    // Set the handle
    pNoti->handle = EEGAttrTbl[EEG_PACK_VALUE_POS].handle;
  
    // Send the notification
    return GATT_Notification( connHandle, pNoti, FALSE );
  }

  return bleIncorrectMode;
}
                               
static uint8 readAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                            uint8 *pValue, uint8 *pLen, uint16 offset, uint8 maxLen )
{
  bStatus_t status = SUCCESS;

  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  uint16 uuid = 0;
  if (utilExtractUuid16(pAttr, &uuid) == FAILURE) {
    // Invalid handle
    *pLen = 0;
    return ATT_ERR_INVALID_HANDLE;
  }

  switch(uuid)
  {
    case EEG_1MV_CALI_UUID:
    case EEG_SAMPLE_RATE_UUID:
      *pLen = 2;
       VOID osal_memcpy( pValue, pAttr->pValue, 2 );
       break;
       
    case EEG_LEAD_TYPE_UUID:
      *pLen = 1;
      pValue[0] = *pAttr->pValue;
      break;
      
    default:
      *pLen = 0;
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  return ( status );
}

static bStatus_t writeAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                 uint8 *pValue, uint8 len, uint16 offset )
{
  bStatus_t status = SUCCESS;
 
  uint16 uuid = 0;
  if (utilExtractUuid16(pAttr,&uuid) == FAILURE) {
    // Invalid handle
    return ATT_ERR_INVALID_HANDLE;
  }
  
  switch ( uuid )
  {
    case GATT_CLIENT_CHAR_CFG_UUID:
      status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                               offset, GATT_CLIENT_CFG_NOTIFY );
      if ( status == SUCCESS )
      {
        uint16 charCfg = BUILD_UINT16( pValue[0], pValue[1] );

        (eegServiceCBs->pfnEegServiceCB)( (charCfg == GATT_CFG_NO_OPERATION) ?
                                EEG_PACK_NOTI_DISABLED :
                                EEG_PACK_NOTI_ENABLED );
      }
      break; 
 
    default:
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }

  return ( status );
}

static void handleConnStatusCB( uint16 connHandle, uint8 changeType )
{ 
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) && 
           ( !linkDB_Up( connHandle ) ) ) )
    { 
      GATTServApp_InitCharCfg( connHandle, eegPackClientCharCfg );
    }
  }
}
