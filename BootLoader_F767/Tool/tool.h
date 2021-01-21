#ifndef __TOOL_H
#define __TOOL_H

#include "main.h"
#include "can_bit.h"


#define MAX(a, b)	(((a)>(b)) ? (a) : (b))
#define MIN(a, b)	(((a)<(b)) ? (a) : (b))

uint8_t float_sort_buff(float *buff, uint16_t size, uint8_t flag);

uint16_t get_average(uint16_t *buff, uint16_t size, uint8_t rm_num);
int get_average_int16(short *buff, uint16_t size, uint8_t rm_num);

uint8_t my_set_bit(void * data, uint8_t bit_index, uint8_t num_bool);
uint16_t my_get_n_bit(uint64_t data, uint8_t bit_index, uint8_t n);

uint8_t sort_buff(uint8_t *buff, uint16_t size, uint8_t flag);
#endif


