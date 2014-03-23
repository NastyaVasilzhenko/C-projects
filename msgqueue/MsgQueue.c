/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified: 24.2.14
Description:  Message Queue over Shared Memory
***************************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <semaphore.h>
#include <sys/msg.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "MsgQueue.h"
#include "Sync.h"
#include "queue.h"
#include "mem.h"
#include "verbose.h"

#define VERBOSE 0
#define KEY_ID 7
#define PERMISSIONS 0666 /*access permissions to file (octal 644:  rw-rw-rw-)*/

/*calculate adress using offsets*/
#define MUTEX(X)				(Sync*)((char*)(X) + ((OffsetTable*)(X))->m_mutexOffset)
#define EMPTY(X)				(Sync*)((char*)(X) + ((OffsetTable*)(X))->m_semEmptyOffset)
#define FULL(X)				(Sync*)((char*)(X) + ((OffsetTable*)(X))->m_semFullOffset)
#define QUEUE(X)			(Queue*)((char*)(X) + ((OffsetTable*)(X))->m_queueOffset)
#define MEMORY(X)			(void*)((char*)(X) + ((OffsetTable*)(X))->m_memPoolOffset)

#define NUM_OF_SYNC_TOOLS 3

/*****************************************************************************
Shared memory segments:

            offsets
------------------------------------
          sync tools
------------------------------------
            queue 
------------------------------------
           msg pool

*****************************************************************************/

/*shared data*/
typedef struct OffsetTable
{
	size_t m_mutexOffset;
	size_t m_semEmptyOffset;
	size_t m_semFullOffset;
	size_t m_queueOffset;
	size_t m_memPoolOffset;
	
}OffsetTable;

/*private data*/
struct MQ
{	
	char* m_sharedMem;	
	Sync* m_mutex;
	Sync* m_empty;
	Sync* m_full;
	Queue* m_queue;
	void* m_memPool;
	int m_id; /*saves the creator's id-used as creator flag*/ 
};

MQ_t*	MsgQueueCreate	(char *_msgQueueName, size_t _msgMax, size_t _memSize)
{
	
	key_t key;
	MQ_t *msgQueue=NULL; 
	OffsetTable* metaData=NULL;
	size_t syncSize, shmSize, queueSize;
	
	assert(_msgQueueName);
	assert(_memSize);
	
	verbPrintf(VERBOSE, "MsgQueueCreate:Allocate private memory");
	msgQueue=(MQ_t*)malloc(sizeof(MQ_t));
	if(!msgQueue)
	{
		return NULL;
	}

	/* calculate size */
	verbPrintf(VERBOSE, "MsgQueueCreate:Calculations");
	syncSize  = SyncGetRequiredSize(); 
	queueSize = QueueGetRequiredSize(_msgMax);
	shmSize = sizeof(OffsetTable) + syncSize*NUM_OF_SYNC_TOOLS + queueSize + _memSize;
		 
	/*crete key*/		 
	verbPrintf(VERBOSE, "MsgQueueCreate: Create key");
	key = ftok(_msgQueueName, KEY_ID);
    if (key  == -1)
   	{
       	perror("ERROR:MsgQueueCreate:ftok");
       	return NULL;
    }
  	/*crete shared segment*/
	verbPrintf(VERBOSE, "MsgQueueCreate: Create shared segment");
	if ((msgQueue->m_id = shmget(key, shmSize, PERMISSIONS | IPC_CREAT | IPC_EXCL)) == -1)
	{
        	perror("ERROR:MsgQueueCreate: shmget");
        	return NULL;
   	}
   
	/*attach message queue to the segment*/
	verbPrintf(VERBOSE, "MsgQueueCreate: Attach shared segment");
   	msgQueue->m_sharedMem = shmat(msgQueue->m_id, (void *)0, 0);
    if ( -1==(int)msgQueue->m_sharedMem)
	{
        	perror("ERROR:MsgQueueCreate: shmat");
        	return NULL;
    }
    
	
	verbPrintf(VERBOSE, "MsgQueueCreate: Create offset table");
	
	metaData=(OffsetTable*)msgQueue->m_sharedMem;
	metaData->m_mutexOffset    = sizeof(OffsetTable);
	metaData->m_semEmptyOffset = metaData->m_mutexOffset+syncSize;
	metaData->m_semFullOffset  = metaData->m_semEmptyOffset+syncSize;
	metaData->m_queueOffset    = metaData->m_semFullOffset + syncSize;
	metaData->m_memPoolOffset  = metaData->m_queueOffset + queueSize;
	
	verbPrintf(VERBOSE, "MsgQueueCreate: Create sync tools");
	
	/*mutex*/

    if(!SyncCreate(msgQueue->m_sharedMem+metaData->m_mutexOffset,1))
	{
		shmdt(msgQueue->m_sharedMem);
		shmctl(msgQueue->m_id, IPC_RMID, NULL);
		free(msgQueue);
		
		perror("ERROR:Mutex creation failed");
		return NULL;
	}
	
	/*sem full*/

	if(!SyncCreate(msgQueue->m_sharedMem+metaData->m_semFullOffset,0))
	{
		
		shmdt(msgQueue->m_sharedMem);
		shmctl(msgQueue->m_id, IPC_RMID, NULL);
		free(msgQueue);
		
		perror("ERROR:Mutex creation failed");
		return NULL;
	}
	
	/*sem empty*/
	
	if(!SyncCreate(msgQueue->m_sharedMem+metaData->m_semEmptyOffset,_msgMax))
	{
		shmdt(msgQueue->m_sharedMem);
		shmctl(msgQueue->m_id, IPC_RMID, NULL);
		free(msgQueue);
				
		perror("ERROR:Mutex creation failed");
		return NULL;
	}
	
	verbPrintf(VERBOSE, "MsgQueueCreate: Initialization");
	 
    msgQueue->m_mutex     = MUTEX(msgQueue->m_sharedMem);
	msgQueue->m_full      = FULL(msgQueue->m_sharedMem);
	msgQueue->m_empty     = EMPTY(msgQueue->m_sharedMem);
 	msgQueue->m_queue     = QueueInit(QUEUE(msgQueue->m_sharedMem),_msgMax);	
	msgQueue->m_memPool   = MyMallocInit(MEMORY(msgQueue->m_sharedMem),_memSize);

	if(!msgQueue->m_memPool)
	{
		QueueDestroy(msgQueue->m_queue);
		shmdt(msgQueue->m_sharedMem);
		shmctl(msgQueue->m_id, IPC_RMID, NULL);
		free(msgQueue);
	}	
	
   	verbPrintf(VERBOSE, "MsgQueueCreate: End");
	return msgQueue;
    
}

MQ_t*	MsgQueueConnect	(char *_msgQueueName)
{
	
	key_t key;
	int shmID;
	MQ_t *msgQueue=NULL; /*private data*/
	
	assert(_msgQueueName);
	
	verbPrintf(VERBOSE, "MsgQueueConnect:Allocate private memory");

	msgQueue=(MQ_t*)malloc(sizeof(MQ_t));
	if(!msgQueue)
	{
		return NULL;
	}
		 
	/*crete key*/		 
	verbPrintf(VERBOSE, "MsgQueueConnect: Create key");
	key = ftok(_msgQueueName, KEY_ID);
    if ( key == -1)
    {
       	perror("ERROR:MsgQueueConnect:ftok");
       	return NULL;
    }	
    
  	/*connect to shared segment*/
	verbPrintf(VERBOSE, "MsgQueueConnect: Connect shared segment");
	if ((shmID = shmget(key, sizeof(OffsetTable), PERMISSIONS)) == -1)
	{
        perror("ERROR:MsgQueue Connect: shmget");
        return NULL;
    }
    
	/*attach message queue to the segment*/
	verbPrintf(VERBOSE, "MsgQueueConnect: Attach shared segment");
	
   	msgQueue->m_sharedMem = shmat(shmID, (void *)0, 0);
    if ( (int)msgQueue->m_sharedMem==-1)
	{
       	perror("ERROR:MsgQueueConnecy: shmat");
       	return NULL;
    }
   
   	verbPrintf(VERBOSE, "MsgQueueConnect: Connect to shared memory utilities");
     
    msgQueue->m_mutex     = MUTEX(msgQueue->m_sharedMem);
	msgQueue->m_full      = FULL(msgQueue->m_sharedMem);
	msgQueue->m_empty     = EMPTY(msgQueue->m_sharedMem);
 	msgQueue->m_queue     = QUEUE(msgQueue->m_sharedMem);	
	msgQueue->m_memPool   = MEMORY(msgQueue->m_sharedMem);	
	msgQueue->m_id = 0;

	verbPrintf(VERBOSE, "MsgQueueConnect: End");
	return msgQueue;
    
}


void	MsgQueueDestroy	(MQ_t *_msgQue)
{

	assert(_msgQue);
	verbPrintf(VERBOSE, "MsgQueueDestroy: Check if creator");
		
	verbPrintf(VERBOSE, "MsgQueueDestroy: Detach");
	shmdt(_msgQue->m_sharedMem);
	
	/*only creator can destroy*/
	if (_msgQue->m_id!=0)
	{
		verbPrintf(VERBOSE, "MsgQueueDestroy: Destroy shared memory");
		/*destroyed only when everyone detaches from it.*/
		shmctl(_msgQue->m_id, IPC_RMID, NULL);
	}

	verbPrintf(VERBOSE, "MsgQueueDestroy: Destroy private data");
	/*destroy private data*/
	free(_msgQue);
	verbPrintf(VERBOSE, "MsgQueueDestroy: End");
}

int	MsgQueueSend (MQ_t *_msgQue, void* _buffer, size_t _length)
{
	char*  	msgAddress = NULL;
	size_t	offset;

	assert(_msgQue);
	assert(_buffer);
	
	verbPrintf(VERBOSE, "MsgQueueSend: Decrease sem empty");
	if(SemDown(_msgQue->m_empty)==-1)
	{
		perror("ERROR:MsgQueueSend:semDown(empty)");
		return 0;
	}

	verbPrintf(VERBOSE, "MsgQueueSend: Message memory allocation");

	if(MutexLock(_msgQue->m_mutex)==-1)
	{
		SemUp(_msgQue->m_empty);
		perror("ERROR:MsgQueueSend:MutexLock");
		return 0;
	}
	
	msgAddress =MyMalloc(_msgQue->m_memPool,sizeof(size_t)+_length);
	
	if(MutexUnlock(_msgQue->m_mutex)==-1)
	{
		perror("ERROR:MsgQueueSend:MutexUnlock");
		return 0;
	}
	if(!msgAddress)
	{
		perror("ERROR:MsgQueueSend:allocation fault");
		return 0;		
	}

	verbPrintf(VERBOSE, "MsgQueueSend: Copy message to memory pool");
	
	/* Copy size of msg to shared memory */
	memcpy(msgAddress , &_length , sizeof(size_t));
	
	/* Copy  msg to shared memory */
	memcpy((char*)msgAddress + sizeof(size_t) , _buffer , _length);

	verbPrintf(VERBOSE, "MsgQueueSend: Calculate msg offset");
	offset = (char*)msgAddress-(char*)(_msgQue->m_memPool);

	verbPrintf(VERBOSE, "MsgQueueSend: Add offset to queue");
	if(MutexLock(_msgQue->m_mutex)==-1)
	{
		perror("ERROR:MsgQueueSend:MutexLock");
		return 0;
	}
	QueueInsert(_msgQue->m_queue, offset);
	
	if(MutexUnlock(_msgQue->m_mutex)==-1)
	{
		perror("ERROR:MsgQueueSend:MutexUnlock");
		return 0;
	}
	
	if(SemUp(_msgQue->m_full)==-1)
	{
		perror("ERROR:MsgQueueSend:semUp(full)");
		return 0;
	}
	verbPrintf(VERBOSE, "MsgQueueSend: End");

	return _length;

}

int	MsgQueueRecv(MQ_t *_msgQue, void* _buffer, size_t _size)
{
	char* 	msgAddress = NULL;
	size_t 	copiedBytes, offset;

	assert(_msgQue);
	assert(_buffer);
	
	verbPrintf(VERBOSE, "MsgQueueRcv: Decrease sem full");
	
	if(SemDown(_msgQue->m_full)==-1)
	{
		perror("ERROR:MsgQueueRcv:semDown(full)");
		return 0;
	}

	verbPrintf(VERBOSE, "MsgQueueRcv: Get offset from queue");
	
	if(MutexLock(_msgQue->m_mutex)==-1)
	{
		SemUp(_msgQue->m_full);
		perror("ERROR:MsgQueueRcv:MutexLock");
		return 0;
	}
	
	QueueRemove(_msgQue->m_queue,(void *)&offset);
	
	if(MutexUnlock(_msgQue->m_mutex)==-1)
	{
		perror("ERROR:MsgQueueRcv:MutexUnlock");
		return 0;
	}

	/* Get message*/
	verbPrintf(VERBOSE, "MsgQueueRcv: Get msg size");
	msgAddress = (char*)(_msgQue->m_memPool) + offset;
	if(!msgAddress)
	{
		perror("ERROR:MsgQueueRcv:Offset calculations");
		return 0;
	}
	memcpy(&copiedBytes ,msgAddress, sizeof(size_t));	
	
	if(copiedBytes > _size)
	{
		/*not enough space for the message data, copies up to available space*/
		copiedBytes=_size;
			
	}
	memcpy(_buffer , msgAddress + sizeof(size_t) ,copiedBytes);	

	verbPrintf(VERBOSE, "MsgQueueRcv: Free memory");
	if(MutexLock(_msgQue->m_mutex)==-1)
	{
		perror("ERROR:MsgQueueRcv:MutexLock");
		return 0;
	}
	MyFree(msgAddress);
	
	if(MutexUnlock(_msgQue->m_mutex)==-1)
	{
		perror("ERROR:MsgQueueRcv:MutexUnlock");
		return 0;
	}

	
	if(SemUp(_msgQue->m_empty)==-1)
	{
		perror("ERROR:MsgQueueSend:semUp(empty)");
		return 0;
	}
	verbPrintf(VERBOSE, "MsgQueueRcv: End");

	return copiedBytes;
}


int	IsMsgQueueEmpty (MQ_t *_msgQue)
{
	assert(_msgQue);
	
	verbPrintf(VERBOSE,"IsMsgQueueEmpty");

	return QueueIsEmpty(_msgQue->m_queue);
}


