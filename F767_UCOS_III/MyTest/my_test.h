
#ifndef MY_TEST_H_
#define MY_TEST_H_

#include "sys.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#define DBG_INFO(fmt,...) printf("%s [%d] -<%s>:"fmt,__FILE__,__LINE__,__FUNCTION__,##__VA_ARGS__);


void buff_test(void);
void left_move(void);

void pn_size_test(void);
void test_struct_pack(void);

void test_priority_level(void);
uint8_t float_sort_buff(float *buff, uint16_t size, uint8_t flag);
void gps_test(void);


int app_test(void);

#endif

