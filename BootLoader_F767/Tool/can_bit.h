

#ifndef __CAN_BIT_H
#define __CAN_BIT_H

#include "main.h"


int moto_lsb_get_bit(uint8_t * buff, uint8_t addr, uint8_t len);
int moto_lsb_pack_bit(uint8_t * buff, uint32_t data, uint8_t addr, uint8_t len);
	

#endif

