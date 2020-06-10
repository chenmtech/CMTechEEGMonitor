/**************************************************************************************************
 * CMTechECGMonitor.h：ECG监视器主程序头文件
 *
 **************************************************************************************************/

#ifndef CMTECHECGMONITOR_H
#define CMTECHECGMONITOR_H


/*********************************************************************
 * 常量
 */


// ECGMonitor任务事件
#define ECGMONITOR_START_DEVICE_EVT             0x0001     // 启动设备事件

// 测量控制标记值
#define ECGMONITOR_CTRL_STOP                    0x00    // 停止测量
#define ECGMONITOR_CTRL_START_ECG               0x01    // 开始测量ECG信号
#define ECGMONITOR_CTRL_START_1MV               0x02    // 开始测量1mV信号

// 电池电量测量控制标记值
#define BATTERY_CTRL_STOP               0x00    // 停止测量
#define BATTERY_CTRL_START              0x01    // 开始测量 


/*********************************************************************
 * 公共函数
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
