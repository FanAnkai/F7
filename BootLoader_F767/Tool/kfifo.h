#ifndef __KFIFO_H
#define __KFIFO_H

#include "stdint.h"
#include "string.h"
#include "stdlib.h"


#define is_power_of_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

typedef struct
{
    unsigned char *buffer;  /* the buffer holding the data*/
    unsigned int size;      /* the size of the allocated buffer*/
    unsigned int in;        /* data is added at offset (in % size)*/
    unsigned int out;       /* data is extracted from off. (out % size)*/
} kfifo;

/**
  * @brief  CAN Rx message structure definition
  */



extern kfifo gFifoRx;
extern kfifo gFifoTx;




void kfifo_init(kfifo *fifo, unsigned char *buffer, unsigned int size);
unsigned int kfifo_put(kfifo *fifo, unsigned char *buffer, unsigned int len);
unsigned int kfifo_get(kfifo *fifo, unsigned char *buffer, unsigned int len);

unsigned int can_fifo_get(kfifo *fifo, unsigned char *buffer, unsigned int len);

void kfifo_reset(kfifo *fifo);
unsigned int kfifo_len(kfifo *fifo);




#endif
