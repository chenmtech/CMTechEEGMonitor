
#include "bcomdef.h"
#include "gatt.h"
#include "gattservapp.h"
#include "App_GATTConfig.h"
#include "Service_ECGMonitor.h"


//����ECGMonitor����
//appCBs: Ӧ�ò�Ϊ�˷����ṩ�Ļص�����
//���������������Ҫ���ã�����մ˺�������һ���º�����
extern void GATTConfig_SetECGMonitorService(ecgServiceCBs_t* appCBs)
{
  ECGMonitor_AddService( GATT_ALL_SERVICES );  // ���ط���  

  // �Ǽǻص�
  VOID ECGMonitor_RegisterAppCBs( appCBs );  
}



//���õ�ص�������
extern void GATTConfig_SetBatteryService(batteryServiceCBs_t* appCBs)
{
  Battery_AddService( GATT_ALL_SERVICES );  // ���ط���  

  // �Ǽǻص�
  VOID Battery_RegisterAppCBs( appCBs );   
}