/**
 * 用户配置文件，最终目标是希望将来使用此bootloader
 * 的用户只需要在此文件修改宏定义（硬件差异就需要增加对应代码）
 * 即可完成配置
 * 
 * 
 * **/


#ifndef __CONFIG_H__
#define __CONFIG_H__


#include "main.h"
#include "stdint.h"
#include "string.h"
/**************************************************/
/**
 * 协议配置
 * **/
 
#define LOCAL_ID        0x4D	//
#define LOCAL_ID_1		0x4E	//
#define LOCAL_ID_2		0x4F	//

#define APP_ID			0x3E	//上位机
#define PC_ID			0x3C	//上位机

#define BLE_ID			0x14	//BLE


//希望兼容哪项协议打开如下定义即可
#define SUPPORT_TO_ONE
//#define SUPPORT_TO_TWO



#define FRAME_MAX (255 + 15)

#define VERSION_HEAD        0x5A

#define OLD_PROTOCOL_VER    0xA5
#define NEW_PROTOCOL_VER    0xB5


#define USE_VERSION		OLD_PROTOCOL_VER



/**************************************************/




#endif

