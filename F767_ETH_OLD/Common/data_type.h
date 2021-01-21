
#ifndef SYS_DATA_TYPE_H_
#define SYS_DATA_TYPE_H_


#include "global.h"


#define COLLECTMAX   20
#define REMOVENUM    2


#define ALARMCOUNT    5
#define PMAlARM       0x01


#define Task_1000ms	1000
#define Task_500ms	500
#define Task_200ms	200
#define Task_100ms	100
#define Task_50ms	50
#define Task_20ms	20
#define Task_10ms	10







typedef struct
{
    u16 PowerUpCnt;

    u16 t_1000ms;
    u16 t_500ms;
    u8  t_200ms;
    u8  t_100ms;
    u8	t_50ms;
    u8  t_20ms;
    u8  t_10ms;

} TASK_TYPE;



typedef struct
{
    u16 u16PM25AlarmLimit;
    u16 u16PM100AlarmLimit;
    u8  u8AutomaticMode ;
    u8  u8ManualMode;
    u16 RelayClosingTime ;
    u8  u8PM25Calibration;
    u8  u8PM10Calibration;
    u8  u8TSPCalibration;

} FactoryCfg_t;



typedef struct
{
    FactoryCfg_t tFactoryCfg;
    u8  TSPEn;
    u8  TSPMode;
} Para_t;



typedef struct
{
    u16 u16PM25;
    u16 u16PM10;
    u16 u16Noise;

    u16 u16WindSpeed;
    u16 u16WindDirection;
    u16 u16AirPressure;
    u16 u16TSP;
    u16 u16PM25AlarmLimit;
    u16 u16PM100AlarmLimit;

    u8  u8Alarm;

} DEV_STAT_TYPE;


extern TASK_TYPE gTask_Rrg;







#endif

