/*
 * struct_size_test.c
 *
 *  Created on: 2020年7月7日
 *      Author: yufei.fan
 */

#include "main.h"

#include "my_test.h"


typedef struct
{
	int		a;
	char	b;
	char	c;
	float	d;
	char	e;
	double	f;

}PN_TYPE;

typedef struct
{
	char a;
	char b;
	double c;
	float g;
	PN_TYPE	pn;
	int i;
	char d;
	char e;
	float f;
	short h;


}TBOX_TYPE;
TBOX_TYPE tbox = {0};
void pn_size_test(void)
{
	int size_pn = sizeof(tbox.pn);
	printf("size_pn = %d\n", size_pn);
}





typedef struct
{
    uint8_t	b1 : 1;
    uint8_t	b2 : 1;
    uint8_t	b3 : 1;
    uint8_t	b4 : 1;

} ERROR_TYPEE;

#pragma pack(1)

typedef struct
{
//    uint32_t	d1 : 8;

//    uint32_t	c1 : 1;
//    uint32_t	c2 : 1;
//    uint32_t	c3 : 1;
//    uint32_t	c4 : 1;
//    
//    uint32_t	d2 : 8;
    
    long long VehicleForcedActivation    :  2;
    int RemoteUnlock               :  1;
//    int EPSWorkingStatusReq        :  2;
//    int EPSWorkingModeReq          :  2;
//    int DriveModeReq               :  2;
//    int DriveModeSpdSet            :  2;
//    int SpeedAdjustmentFunc        :  4;
//    int Remaining_mileage          : 16;
//    int BluetoothPowerOn		   :  2;
//    int RemotePowerOn              :  2;

} TBOX_PACK1;

#pragma pack()

typedef struct
{
//    uint32_t	d1 : 8;

//    uint32_t	c1 : 1;
//    uint32_t	c2 : 1;
//    uint32_t	c3 : 1;
//    uint32_t	c4 : 1;
//    
//    uint32_t	d2 : 8;
    
    long long VehicleForcedActivation    :  2;
    int RemoteUnlock               :  1;
//    int EPSWorkingStatusReq        :  2;
//    int EPSWorkingModeReq          :  2;
//    int DriveModeReq               :  2;
//    int DriveModeSpdSet            :  2;
//    int SpeedAdjustmentFunc        :  4;
//    int Remaining_mileage          : 16;
//    int BluetoothPowerOn		   :  2;
//    int RemotePowerOn              :  2;

} TBOX_UNPACK;


TBOX_PACK1  tbox_pack1 = {0};
TBOX_UNPACK tbox_unpack = {0};

void test_struct_pack(void)
{
    uint32_t temp = 0xaaaabbbb;

    memcpy(&tbox_pack1, &temp, 4);
    memcpy(&tbox_unpack, &temp, 4);

    printf("sizeof(tbox_pack1) = %d\r\n", sizeof(tbox_pack1));
    printf("sizeof(tbox_unpack) = %d\r\n", sizeof(tbox_unpack));

}





