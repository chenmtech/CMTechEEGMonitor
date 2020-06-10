//Copyright (C), ChenMing, GDMC
//CMFIRDESIGN.H：本模块用于实现心电信号滤除直流成分（包括呼吸信号等低频干扰） ，以及50Hz工频干扰的滤波器 



//采用的滤波器DCAND50HZ_Filter是一个线性相位的类型1的FIR低通滤波器，对h(n)插入零值后，实现频谱尺度收缩，达到超低带宽的效果 
//具体算法可参见杨福生的《医学信号处理》
//written by chenm, 2016-05-24 

//采用的滤波器DCAND50HZ_IntegerFilter是一个整系数的滤波器，1Hz截止频率有-2.0dB左右衰减
//具体算法可参见杨福生的《医学信号处理》
//written by chenm, 2016-05-30 


#ifndef DCAND50HZFilter_H
#define DCAND50HZFilter_H


extern int DCAND50HZ_IntegerFilter(int x);

//extern double DCAND50HZ_Filter(double x);

#endif

 



