#include "ql_queue.h"
#include "ql_include.h"

ql_queue *create_ql_queue(type_u32 itemsize, type_u32 itemcount)
{
	ql_queue *queue = (ql_queue *)ql_malloc(sizeof(ql_queue));
	if (NULL == queue) {
		return NULL;
	}

	queue->item_size = itemsize;
	queue->item_count = itemcount;

	queue->data = ql_malloc((itemsize * itemcount)*sizeof(char));
	if (NULL == queue->data) {
		ql_free(queue);
		return NULL;
	}
	ql_memset(queue->data, 0x00, (itemsize * itemcount)*sizeof(char));

    queue->head = 0;
    queue->tail = 0;

    queue->queue_mutex = ql_mutex_create();
    if (NULL == queue->queue_mutex) {
    	ql_free(queue->data);
    	ql_free(queue);
    	return NULL;
    }

    return queue;
}

void destroy_ql_queue(ql_queue **queue)
{
	if (queue && *queue) {
		if ((*queue)->data) {
			ql_free((*queue)->data);
			(*queue)->data = NULL;
		}

		if ((*queue)->queue_mutex) {
			ql_mutex_destroy(&((*queue)->queue_mutex));
		}

		ql_free(*queue);
		*queue = NULL;
	}
}

int is_empty_ql_queue(ql_queue *queue)
{
	int ret = 0;

	if (NULL == queue) {
		return ret;
	}

	if (ql_mutex_lock(queue->queue_mutex)) {
		if ((queue->head == queue->tail)) {
			ret = 1;
		} else {
			ret = 0;
		}
		ql_mutex_unlock(queue->queue_mutex);
	} else {
	}

	return ret;
}

int is_full_ql_queue(ql_queue *queue)
{
	int ret = 0;

	if (NULL == queue) {
		return ret;
	}

	if (ql_mutex_lock(queue->queue_mutex)) {
		if (((queue->head + 1) % queue->item_count) == queue->tail) {
			ret = 1;
		} else {
			ret = 0;
		}
		ql_mutex_unlock(queue->queue_mutex);
	}

	return ret;
}

int en_ql_queue(ql_queue *queue, void *item, type_u32 itemsize)
{
	if (item == NULL || itemsize != queue->item_size) {
		return -1;
	}

    if (is_full_ql_queue(queue)) {
    	return -1;
    } else {
    	if (ql_mutex_lock(queue->queue_mutex)) {

    		ql_memcpy((void *)(((char *)queue->data)+(queue->head*queue->item_size)), item, queue->item_size);
    		queue->head = (queue->head+1) % queue->item_count;

    		ql_mutex_unlock(queue->queue_mutex);
    	} else {
    		return -1;
    	}
     }

    return 0;
}

int de_ql_queue(ql_queue *queue, void *item, type_u32 itemsize)
{
	if (item == NULL || itemsize != queue->item_size) {
		return -1;
	}

    if (is_empty_ql_queue(queue)) {
    	return -1;
    }

    if (ql_mutex_lock(queue->queue_mutex)) {

    	ql_memcpy(item, (void *)(((char *)queue->data)+(queue->tail*queue->item_size)), itemsize);
    	queue->tail = (queue->tail+1)%queue->item_count;

    	ql_mutex_unlock(queue->queue_mutex);
    }

	return 0;
}

int get_ql_queue(ql_queue *queue, void *item, type_u32 itemsize)
{
	if (item == NULL || itemsize != queue->item_size) {
		return -1;
	}

    if (is_empty_ql_queue(queue)) {
    	return -1;
    }

    if (ql_mutex_lock(queue->queue_mutex)) {

    	ql_memcpy(item, (void *)(((char *)queue->data)+(queue->tail*queue->item_size)), itemsize);

    	ql_mutex_unlock(queue->queue_mutex);
    }

	return 0;
}
