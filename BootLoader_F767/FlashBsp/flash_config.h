#ifndef __FLASH_CONFIG_H__
#define __FLASH_CONFIG_H__

#include <stdint.h>
#include "stm32f7xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (STM32F767xx)
    #define NBOOT_FLASH_START_ADRESS    ((uint32_t)0x08000000)
    #define NBOOT_FLASH_SIZE            (2 * 1024 * 1024)
    #define NBOOT_FLASH_END_ADDRESS     ((uint32_t)(NBOOT_FLASH_START_ADRESS + NBOOT_FLASH_SIZE))

    #define NBOOT_SRAM_SIZE             512
    #define NBOOT_SRAM_END              (0x20000000 + NBOOT_SRAM_SIZE * 1024)
#endif
    
#ifdef __cplusplus
}
#endif

#endif /* __FLASH_CONFIG_H__ */
