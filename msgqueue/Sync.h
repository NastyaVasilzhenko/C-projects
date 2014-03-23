/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified:24.2.14
Description:  Sync tools. 
***************************************************************************************/

#ifndef __SYNC_H_
#define __SYNC_H_

#include <semaphore.h>
#include <sys/types.h>

typedef struct  Sync  Sync;

Sync* SyncCreate (void*  _address, size_t _initVal);

void SyncDestroy (Sync*  _sync);

int	SemUp (Sync* _sem);

int	MutexUnlock (Sync* _mutex);

int	SemDown	(Sync*  _sem);

int	MutexLock(Sync* _mutex);

size_t SyncGetRequiredSize();

size_t SemGetVal(Sync* _sem);


#endif /* __SYNC_H_ */
