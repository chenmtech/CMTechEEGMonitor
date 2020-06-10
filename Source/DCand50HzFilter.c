
#include "dcand50hzfilter.h"


//这个滤波器用的是一个整系数的（全通网络-二阶梳状滤波器）
//低频段的截止频率为1Hz
//为什么不做低频段截止频率为0.5Hz的滤波呢？因为发现有一定的低频漂移没有被滤除
//而且50Hz工频干扰较大，有可能就是所谓的工频频率漂移，当滤波器带宽太过狭窄时，滤除不了漂移的工频干扰
//起码现在的实验说明了这一点
//2016-05-30
static int x1buff[161] = {0};
static long y1buff[6] = {0};
static int x2buff[161] = {0};
static long y2buff[6] = {0};
static int * pX1First = &x1buff[0];
static int * pX1Last = &x1buff[160];
static long * pY1First = &y1buff[0]; 
static long * pY1Last = &y1buff[5];
static int * pX1Begin = &x1buff[0];
static int * pX1End = &x1buff[160];
static int * pX1315 = &x1buff[155];
static long * pY1Begin = &y1buff[0];
static long * pY1End = &y1buff[5];
static int out1 = 0;

static int * pX2First = &x2buff[0];
static int * pX2Last = &x2buff[160];
static long * pY2First = &y2buff[0];
static long * pY2Last = &y2buff[5];
static int * pX2Begin = &x2buff[0];
static int * pX2End = &x2buff[160];
static long * pY2Begin = &y2buff[0];
static long * pY2End = &y2buff[5];
static long out2 = 0;
#define RSHIFT 5

extern int DCAND50HZ_IntegerFilter(int x)
{
	(pX1Begin == pX1First) ? pX1Begin = pX1Last : pX1Begin--; 
	(pX1End == pX1First) ? pX1End = pX1Last : pX1End--;
	*pX1Begin = x;
	(pX1315 == pX1First) ? pX1315 = pX1Last : pX1315--;
	
	(pY1Begin == pY1First) ? pY1Begin = pY1Last : pY1Begin--;
	(pY1End == pY1First) ? pY1End = pY1Last : pY1End--;
	
	out1 = (*pY1Begin = *pY1End + x - *pX1End)>>RSHIFT;
	
	(pX2Begin == pX2First) ? pX2Begin = pX2Last : pX2Begin--; 
	(pX2End == pX2First) ? pX2End = pX2Last : pX2End--;
	*pX2Begin = out1;
	
	(pY2Begin == pY2First) ? pY2Begin = pY2Last : pY2Begin--;
	(pY2End == pY2First) ? pY2End = pY2Last : pY2End--;
	
	out2 = *pY2Begin = *pY2End + out1 - *pX2End;
	return (int)(*pX1315 - (int)(out2>>RSHIFT));
	
}

/*
#define N2 55
#define halfN2 27
#define M2 271
#define halfM2 135

static const double hn[halfN2+1] = {-0.000925669341136,-0.001085888170863,-0.001346709095894,-0.001733828653661,-0.002270729200571,-0.002977727041780,-0.003871092690452,-0.004962275036627,-0.006257258298873,-0.007756076677378,-0.009452506749694,-0.011333952008977,-0.013381527726875,-0.015570347739060,-0.017870008025672,-0.020245255323519,-0.022656822692408,-0.025062408185956,-0.027417767751467,-0.029677889383469,-0.031798212529266,-0.033735854904044,-0.035450808288294,-0.036907065579055,-0.038073643331369,-0.038925467196084,-0.039444091931049,0.960381767106985};
static double xn[M2] = {0.0};

static const double * pH0 = 0;
static const double * pH = 0;
static double * pX0 = 0;
static double * pX01 = 0;
static double * pX = 0;
static const int I = 5;
static double y = 0.0;
static double tmp = 0.0;
static double tmp1 = 0.0;

extern double DCAND50HZ_Filter(double x)
{
        pH0 = &hn[0];
        pH = &hn[halfN2];
        pX0 = &xn[halfM2];
        pX01 = pX0-1;
        pX = pX0+1;   
  
	tmp = *pX0;
	y = *pH-- * (*pX0-- = *pX01--);		//先算中间一项
        
	int i = 0;
	while(pH >= pH0)
	{
		if(++i != I)
		{
			*pX0-- = *pX01--;
			tmp1 = tmp, tmp = *pX, *pX++ = tmp1;
		}
		else
		{
			if(pH == pH0) break;

			y += *pH-- * ( (*pX0-- = *pX01--) + (tmp1 = tmp, tmp = *pX, *pX++ = tmp1) );		//偶对称，两项相加
			i = 0;		
		}
	} 

	return y + *pH * ( (*pX0 = x) + (*pX = tmp) );	
}
*/








