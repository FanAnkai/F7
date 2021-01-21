/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-12-22     zylx         first version
 */

#include "flash_config.h"
#include "mcu_flash.h"
#include "stm32g0xx.h"

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t addr)
{
    uint32_t page = 0;
    //page = RT_ALIGN_DOWN(addr, FLASH_PAGE_SIZE);
    page = (addr) & ~((FLASH_PAGE_SIZE) - 1);
    return page;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
int stm32_flash_read(uint32_t addr, uint8_t *buf, uint32_t size)
{
    uint32_t i;

    if ((addr + size) > NBOOT_FLASH_END_ADDRESS)
    {
        return -ERROR_FLASH_OUT_OF_RANGE;
    }

    for (i = 0; i < size; i++, buf++, addr++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
int stm32_flash_write(uint32_t addr, const uint8_t *buf, uint32_t size)
{
    int result        = ERROR_FLASH_OK;
    uint32_t end_addr   = addr + size;

    if (addr % 8 != 0)
    {
        return -ERROR_FLASH_MISALIGN;
    }

    if ((end_addr) > NBOOT_FLASH_END_ADDRESS)
    {
        return -ERROR_FLASH_OUT_OF_RANGE;
    }

    HAL_FLASH_Unlock();

    while (addr < end_addr)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, *((uint64_t *)buf)) == HAL_OK)
        {
            if (*(uint64_t *)addr != *(uint64_t *)buf)
            {
                result = -ERROR_FLASH_PROGRAM_FAIL;
                break;
            }
            addr += 8;
            buf  += 8;
        }
        else
        {
            result = -ERROR_FLASH_PROGRAM_FAIL;
            break;
        }
    }

    HAL_FLASH_Lock();

    if (result != ERROR_FLASH_OK)
    {
        return result;
    }

    return size;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
int stm32_flash_erase(uint32_t addr, uint32_t size)
{
    int result = ERROR_FLASH_OK;
    uint32_t PAGEError = 0;

    /*Variable used for Erase procedure*/
    FLASH_EraseInitTypeDef EraseInitStruct;

    if ((addr + size) > NBOOT_FLASH_END_ADDRESS)
    {
        return -ERROR_FLASH_OUT_OF_RANGE;
    }

    HAL_FLASH_Unlock();

    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page        = GetPage(addr);
    EraseInitStruct.NbPages     = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    {
        result = -ERROR_FLASH_ERASE_FAIL;
        goto __exit;
    }

__exit:
    HAL_FLASH_Lock();

    if (result != ERROR_FLASH_OK)
    {
        return result;
    }

    return size;
}
