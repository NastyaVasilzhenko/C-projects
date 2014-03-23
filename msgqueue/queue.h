/****************************
Author:Anastasia Vasilzhenko
Date created:11.11.13
Date modified:19.02.14
Description: Circular Queue.Fifo.
***************************/

#ifndef _QUEUE_H__
#define _QUEUE_H__

#include "adterr.h"

typedef struct Queue Queue;

typedef void* Data;


Queue* QueueCreate(size_t _size);

/*Intialize queue of _size items at _address	*/
Queue* QueueInit(void* _address, size_t _size);


void QueueDestroy (Queue* _queue);


ADTERR QueueInsert (Queue* _queue, int _data);


size_t QueueGetRequiredSize (size_t _nElements);

ADTERR QueueRemove (Queue* _queue, int* _data);


int QueueIsEmpty (Queue* _queue);

int QueueIsFull (Queue* _queue);

size_t QueueGetItemsNum (Queue* _queue);

#endif   /*endif __QUEUE_H__ */

