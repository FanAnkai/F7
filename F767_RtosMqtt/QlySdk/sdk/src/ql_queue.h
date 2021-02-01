#ifndef QL_QUEUE_H_
#define QL_QUEUE_H_
#include <stdint.h>
#include "ql_include.h"

typedef struct _ql_queue
{
    type_u32 item_size;
    type_u32 item_count;
    type_u32 head;
    type_u32 tail;
    void  *data;
    ql_mutex_t *queue_mutex;
} ql_queue;


ql_queue *create_ql_queue(type_u32 itemsize, type_u32 itemcount);

void destroy_ql_queue(ql_queue **queue);

int is_empty_ql_queue(ql_queue *queue);

int is_full_ql_queue(ql_queue *queue);

int en_ql_queue(ql_queue *queue, void *item, type_u32 itemsize);

int de_ql_queue(ql_queue *queue, void *item, type_u32 itemsize);

int get_ql_queue(ql_queue *queue, void *item, type_u32 itemsize);


#endif /* QL_QUEUE_H_ */
