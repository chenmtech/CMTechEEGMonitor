
/*
* Service_ECGMonitor.h : ECG监视器服务头文件
* Written by chenm, 2017-04-13
*/


#ifndef SERVICE_ECGMONITOR_H
#define SERVICE_ECGMONITOR_H

#include "bcomdef.h"

/*********************************************************************
 * 常数
 */

// ECGMonitor服务特征标记
#define ECGMONITOR_DATA                  0   //ECG导联数据
#define ECGMONITOR_CTRL                  1   //测量控制点
#define ECGMONITOR_1MV                   2   //1mV定标值
#define ECGMONITOR_SAMPLERATE            3   //采样率
#define ECGMONITOR_LEADTYPE              4   //导联类型
 

// ECGMonitor服务和特征的UUID
#define ECGMONITOR_SERV_UUID               0xAA40  //ECGMonitor服务UUID
#define ECGMONITOR_DATA_UUID               0xAA41  //ECG信号值特征UUID
#define ECGMONITOR_CTRL_UUID               0xAA42  //采样控制点UUID
#define ECGMONITOR_1MV_UUID                0xAA43  //1mV定标值特征UUID
#define ECGMONITOR_SAMPLERATE_UUID         0xAA44  //采样频率UUID
#define ECGMONITOR_LEADTYPE_UUID           0xAA45  //ECG导联类型UUID


//导联类型
#define LEADTYPE_I            0x00    //Lead I
#define LEADTYPE_II           0x01    //Lead II
#define LEADTYPE_III          0x02    //Lead III


  
// ECG Monitor Service bit fields
#define ECGMONITOR_SERVICE                 0x00000001

// 每个ECG数据包的字节长度
#define ECG_PACKET_LEN 20



/*********************************************************************
 * 类型声明
 */

// 需要应用层提供的回调函数声明
// 当特征发生变化时，通知应用层
typedef NULL_OK void (*ecgServiceAppCB_t)( uint8 paramID );

// 需要应用层提供的回调结构体声明
typedef struct
{
  ecgServiceAppCB_t        pfnEcgServiceCB;
} ecgServiceCBs_t;

  



/*********************************************************************
 * 公共函数 
 */

//加载服务
extern bStatus_t ECGMonitor_AddService( uint32 services );

//登记应用层回调结构体实例
extern bStatus_t ECGMonitor_RegisterAppCBs( ecgServiceCBs_t * appCallbacks );

//让应用层设置服务的特征参数
extern bStatus_t ECGMonitor_SetParameter( uint8 param, uint8 len, void *value );

//让应用层获取指定的特征参数
extern bStatus_t ECGMonitor_GetParameter( uint8 param, void *value );

// 返回ecgData给外部数据处理模块，免去重复分配数据空间
extern uint8 * ECGMonitor_GetECGDataPointer();

//发送一个ECG信号值的Notification
//extern bStatus_t CMTechECGMonitor_ECGSignalNotify( uint16 connHandle, attHandleValueNoti_t *pNoti );



/*********************************************************************
*********************************************************************/


#endif 
