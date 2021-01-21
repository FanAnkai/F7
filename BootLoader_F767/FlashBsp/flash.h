/**
 * @file flash.h
 * @author Segway Inside 
 * @brief STM32 片上 FLASH 操作接口函数
 * @version 1.0
 * @date 2020-01-21
 * 
 * @copyright Copyright (c) 2020 纳恩博（北京）科技有限公司
 * All rights reserved.
 * 
 */

#ifndef __FLASH_H_
#define __FLASH_H_

#include <stdint.h>
#include <string.h>
#include "flash_config.h"

/**
 * @brief 按字节读取FLASH数据
 * 
 * @param addr 数据读取起始地址
 * @param buf 指向待读取数据的buffer指针
 * @param size 待读取数据大小，以字节为单位
 * @return int 
 *              ERROR_FLASH_MISALIGN        地址未对齐
 *              ERROR_FLASH_OUT_OF_RANGE    地址超过了FLASH有效数据范围
 *              > 0                         实际读取的字节数
 */
int flash_read(uint32_t addr, uint8_t *buf, uint32_t size);

/**
 * @brief 按字节写入FLASH数据
 * 
 * @param addr 数据写入起始地址
 * @param buf 指向待写入数据的buffer指针
 * @param size 待写入数据大小，以字节为单位
 * @return int 
 *              ERROR_FLASH_MISALIGN        地址未对齐
 *              ERROR_FLASH_OUT_OF_RANGE    地址超过了FLASH有效数据范围
 *              ERROR_FLASH_PROGRAM_FAIL    数据写入发生错误
 *              > 0                         实际写入的字节数
 */
int flash_write(uint32_t addr, const uint8_t *buf, uint32_t size);

/**
 * @brief 擦除指定地址开始的单个/多个SECTOR
 * 
 * @param addr 擦除起始地址
 * @param size 擦除大小，以字节为单位
 * @return int 
 *              ERROR_FLASH_MISALIGN        地址未对齐
 *              ERROR_FLASH_OUT_OF_RANGE    地址超过了FLASH有效数据范围
 *              ERROR_FLASH_ERASE_FAIL      擦除发生错误
 *              > 0                         实际擦除大小
 */
int flash_erase(uint32_t addr, uint32_t size);


#endif
