#ifndef __FLASH_UTILS_H_
#define __FLASH_UTILS_H_

#include <stdint.h>

typedef int (*flash_read_func)(uint32_t addr, uint8_t *buf, uint32_t size);
typedef int (*flash_write_func)(uint32_t addr, const uint8_t *buf, uint32_t size);
typedef int (*flash_erase_func)(uint32_t addr, uint32_t size);

typedef struct {
    flash_read_func read;
    flash_write_func write;
    flash_erase_func erase;
} Flash_Backend_Type;

#endif


