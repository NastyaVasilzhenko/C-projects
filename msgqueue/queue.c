/********************************
Author:Anastasia Vasizlhenko
Date created:11.11.13
Date modified:11.11.13
Description:
Circular Queue.Fifo.
********************************/

#include <stdio.h>
#include <stdlib.h>

#include "adterr.h"
#include "queue.h"

struct Queue
{
	size_t m_size;
	size_t m_nItems;
	size_t m_head; 
	size_t m_tail; 
	int m_data[1];
};

Queue* QueueCreate(size_t _size)
{
	Queue* queue;
	
	if (_size == 0)
	{
		return NULL;
	}

	queue=(Queue*) malloc (sizeof(Queue) + sizeof(int) * (_size - 1));   
	if (!queue)
	{
		return NULL;
	}
	
	return QueueInit (queue, _size);	
}

Queue* QueueInit(void* _address, size_t _size)
{
	Queue* queue = (Queue*)_address;
	
	queue->m_size=_size;
	queue->m_nItems=0;
	queue->m_head=0;
	queue->m_tail=0;

	return queue;
}

void QueueDestroy (Queue* _queue)
{
	if (_queue)
	{
		free(_queue->m_data);
		free(_queue);
	}
}

size_t QueueGetRequiredSize (size_t _nElements)
{
	return ( sizeof (Queue) + sizeof(int) * (_nElements - 1));
}


ADTERR QueueInsert (Queue* _queue, int _data)
{
	if(!_queue)
	{
		return ERR_NOT_INITILIZED;
	}

	if (_queue->m_nItems == _queue->m_size)
	{
		return ERR_OVERFLOW;
	}

	_queue->m_data[_queue->m_tail]=_data;
	++_queue->m_nItems;

	if (_queue->m_tail == _queue->m_size-1)
	{
		_queue->m_tail=0;
	}
	else
	{
		++_queue->m_tail;
	}
	
	return ERR_OK;
}

ADTERR QueueRemove (Queue* _queue, int* _data)
{
	if(!_queue)
	{
		return ERR_NOT_INITILIZED;
	}

	if (!_queue->m_nItems)
	{
		return ERR_UNDERFLOW;
	}

	*_data = _queue->m_data[_queue->m_head];
	--_queue->m_nItems;

	if (_queue->m_head == (_queue->m_size-1))
	{
		_queue->m_head=0;
	}
	else
	{
		++_queue->m_head;
	}

	return ERR_OK;
}

int QueueIsEmpty (Queue* _queue)
{
	if(!_queue)
	{	
		return 1;
	}

	return (_queue->m_nItems == 0) ? 1 : 0;
}


int QueueIsFull (Queue* _queue)
{
	if(!_queue)
	{	
		return 1;
	}
	
	return (_queue->m_nItems == _queue->m_size) ? 1 : 0;
}

size_t QueueGetItemsNum (Queue* _queue)
{
	if(!_queue)
	{	
		return 0;
	}
	
	return _queue->m_nItems;
}

	
