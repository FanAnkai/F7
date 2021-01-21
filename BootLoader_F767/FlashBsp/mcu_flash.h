#ifndef __MCU_FLASH_H_
#define __MCU_FLASH_H_

#include <stdint.h>

#define RT_EOK      0
#define RT_ERROR    1
#define RT_EINVAL   2

#define LOG_D(...)
#define LOG_E(...)

int stm32_flash_read(uint32_t addr, uint8_t *buf, uint32_t size);
int stm32_flash_write(uint32_t addr, const uint8_t *buf, uint32_t size);
int stm32_flash_erase(uint32_t addr, uint32_t size);

#endif

