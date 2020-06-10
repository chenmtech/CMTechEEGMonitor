
/*
* Service_ECGMonitor.h : ECG����������ͷ�ļ�
* Written by chenm, 2017-04-13
*/


#ifndef SERVICE_ECGMONITOR_H
#define SERVICE_ECGMONITOR_H

#include "bcomdef.h"

/*********************************************************************
 * ����
 */

// ECGMonitor�����������
#define ECGMONITOR_DATA                  0   //ECG��������
#define ECGMONITOR_CTRL                  1   //�������Ƶ�
#define ECGMONITOR_1MV                   2   //1mV����ֵ
#define ECGMONITOR_SAMPLERATE            3   //������
#define ECGMONITOR_LEADTYPE              4   //��������
 

// ECGMonitor�����������UUID
#define ECGMONITOR_SERV_UUID               0xAA40  //ECGMonitor����UUID
#define ECGMONITOR_DATA_UUID               0xAA41  //ECG�ź�ֵ����UUID
#define ECGMONITOR_CTRL_UUID               0xAA42  //�������Ƶ�UUID
#define ECGMONITOR_1MV_UUID                0xAA43  //1mV����ֵ����UUID
#define ECGMONITOR_SAMPLERATE_UUID         0xAA44  //����Ƶ��UUID
#define ECGMONITOR_LEADTYPE_UUID           0xAA45  //ECG��������UUID


//��������
#define LEADTYPE_I            0x00    //Lead I
#define LEADTYPE_II           0x01    //Lead II
#define LEADTYPE_III          0x02    //Lead III


  
// ECG Monitor Service bit fields
#define ECGMONITOR_SERVICE                 0x00000001

// ÿ��ECG���ݰ����ֽڳ���
#define ECG_PACKET_LEN 20



/*********************************************************************
 * ��������
 */

// ��ҪӦ�ò��ṩ�Ļص���������
// �����������仯ʱ��֪ͨӦ�ò�
typedef NULL_OK void (*ecgServiceAppCB_t)( uint8 paramID );

// ��ҪӦ�ò��ṩ�Ļص��ṹ������
typedef struct
{
  ecgServiceAppCB_t        pfnEcgServiceCB;
} ecgServiceCBs_t;

  



/*********************************************************************
 * �������� 
 */

//���ط���
extern bStatus_t ECGMonitor_AddService( uint32 services );

//�Ǽ�Ӧ�ò�ص��ṹ��ʵ��
extern bStatus_t ECGMonitor_RegisterAppCBs( ecgServiceCBs_t * appCallbacks );

//��Ӧ�ò����÷������������
extern bStatus_t ECGMonitor_SetParameter( uint8 param, uint8 len, void *value );

//��Ӧ�ò��ȡָ������������
extern bStatus_t ECGMonitor_GetParameter( uint8 param, void *value );

// ����ecgData���ⲿ���ݴ���ģ�飬��ȥ�ظ��������ݿռ�
extern uint8 * ECGMonitor_GetECGDataPointer();

//����һ��ECG�ź�ֵ��Notification
//extern bStatus_t CMTechECGMonitor_ECGSignalNotify( uint16 connHandle, attHandleValueNoti_t *pNoti );



/*********************************************************************
*********************************************************************/


#endif 
