/**
 * @file nboot.h
 * @author Segway Inside 
 * @brief 和 IAP 固件升级有关的函数
 * @version 0.2
 * @date 2020-03-06
 * 
 * @copyright Copyright (c) 2020 纳恩博（北京）科技有限公司
 * All rights reserved.
 * 
 */
#ifndef __NBOOT_H_
#define __NBOOT_H_

#include <stdint.h>
#include "flash_utils.h"

enum {
    IMAGE_STATE_INVALID = 0,
    IMAGE_STATE_DOWNLOAD_OK,
};

#define IMAGE_DOWN_MAGIC            0xa5389ace

//#define RTT_ENABLE
//#define NBOOT_DEBUG

#define IMAGE_META_DATA_ADDR        0x08008000
#define IMAGE_META_DATA_MAX_SIZE    0x4000
#define IMAGE_SLOT_A_START          0x08010000
#define MAX_IMAGE_SIZE              ((32+32+128+256*3)*1024)//960k

#define IMAGE_SLOT_A_END            (IMAGE_SLOT_A_START+MAX_IMAGE_SIZE-1)
#define IMAGE_SLOT_B_START          (IMAGE_SLOT_A_END+1)
#define IMAGE_SLOT_B_END            (IMAGE_SLOT_B_START+MAX_IMAGE_SIZE-1)


typedef struct {
    uint32_t start;
    uint32_t end;
} Slot_Range;

typedef struct {
    uint32_t magic_bytes;
    uint8_t down_state;
    uint32_t down_size;
} IAP_Images_Down_State;

typedef struct {
    uint32_t flash_start_addr;
    Flash_Backend_Type *backend;
} Temp_Flash_Context;

/**
 * @brief 获取固件下载存储区，固件下载之前调用（仅适用于片上FLASH）
 * 
 * @return const Slot_Range*    指向固件下载区地址信息
 */
const Slot_Range *nboot_get_down_slot(void);

/**
 * @brief 获取固件下载状态
 * 
 * @param down_state        用于获取固件下载状态的指针
 * @param down_size         用于获取固件大小的指针
 * @return int 
 *                  0       获取成功
 *                  < 0     获取失败
 */
int nboot_get_image_down_state(uint8_t *down_state, uint32_t *down_size);

/**
 * @brief 设置固件下载状态
 * 
 * @param down_state        固件下载状态
 * @param down_size         固件大小
 * @return int 
 *                  0       设置成功
 *                  < 0     设置失败
 */
int nboot_set_image_down_state(uint8_t down_state, uint32_t down_size);

/**
 * @brief 启动应用程序（如果有新固件，先拷贝固件）
 * 
 * @param flash_ctx         临时区（固件下载区）Flash操作上下文
 */
void boot_app(const Temp_Flash_Context *flash_ctx);

#endif
