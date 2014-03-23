/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified:24.2.14
Description:  Sync tools-POSIX semaphore based. 
Mutex implemented using semaphore with initial value 1.
Double unlock on mutex-undefined behevior.
***************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>

#include "Sync.h"
#include "verbose.h"

#define VERBOSE 1
#define SHARED 1

struct  Sync
{
	sem_t m_sem;
};

Sync* SyncCreate(void*  _address, size_t _initVal)
{
	Sync* newSem=NULL;
	
	assert(_address);
	
	verbPrintf(VERBOSE, "SemCreate:initialize semaphore");
	
	newSem=_address;
	if (sem_init(&newSem->m_sem,SHARED,_initVal)==-1)
	{
		perror("SemCreate: sen_init");
		sem_destroy(&newSem->m_sem);
		return NULL;
	}
	return newSem;

}

void	SyncDestroy(Sync*  _sync)
{
	sem_destroy(&_sync->m_sem);
}

int	SemUp(Sync* _sem)
{
	assert(_sem); 

	return sem_post(&_sem->m_sem);
}

int MutexUnlock(Sync* _mutex)
{

	assert(_mutex); 

	return sem_post(&_mutex->m_sem);
}

int	SemDown	(Sync*  _sem)
{
	assert(_sem);

	return sem_wait(&_sem->m_sem);	
}

int MutexLock(Sync* _mutex)
{
	assert(_mutex);

	return sem_wait(&_mutex->m_sem);	
}

size_t SyncGetRequiredSize()
{
	return sizeof(Sync);
}

size_t SemGetVal(Sync* _sem)
{
	int val;
	assert(_sem);

	if(sem_getvalue(&_sem->m_sem,&val)==-1)
	{
		return 0;
	}
	
	return val;
}
