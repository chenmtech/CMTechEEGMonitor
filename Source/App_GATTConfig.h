/**
* ���ģ�����ṩ����Ӧ�÷�������ù���
* ��ͬӦ�÷�����Ҫ������Ӧ�����ú���
*/
#ifndef APP_GATTCONFIG_H
#define APP_GATTCONFIG_H


#include "Service_ECGMonitor.h"

#include "Service_Battery.h"

//����ECGMonitor����
extern void GATTConfig_SetECGMonitorService(ecgServiceCBs_t* appCBs);


//���õ�ص�������
extern void GATTConfig_SetBatteryService(batteryServiceCBs_t* appCBs);






#endif