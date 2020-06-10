/**************************************************************************************************
 * CMTechECGMonitor.h��ECG������������ͷ�ļ�
 *
 **************************************************************************************************/

#ifndef CMTECHECGMONITOR_H
#define CMTECHECGMONITOR_H


/*********************************************************************
 * ����
 */


// ECGMonitor�����¼�
#define ECGMONITOR_START_DEVICE_EVT             0x0001     // �����豸�¼�

// �������Ʊ��ֵ
#define ECGMONITOR_CTRL_STOP                    0x00    // ֹͣ����
#define ECGMONITOR_CTRL_START_ECG               0x01    // ��ʼ����ECG�ź�
#define ECGMONITOR_CTRL_START_1MV               0x02    // ��ʼ����1mV�ź�

// ��ص����������Ʊ��ֵ
#define BATTERY_CTRL_STOP               0x00    // ֹͣ����
#define BATTERY_CTRL_START              0x01    // ��ʼ���� 


/*********************************************************************
 * ��������
 */

/*
 * Task Initialization for the BLE Application
 */
extern void ECGMonitor_Init( uint8 task_id );

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 ECGMonitor_ProcessEvent( uint8 task_id, uint16 events );


//extern void ECGMonitor_SendECGSignals(uint8* pData, uint8 len);

/*********************************************************************
*********************************************************************/



#endif 
