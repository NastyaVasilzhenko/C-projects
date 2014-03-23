#include <assert.h>
#include <stdio.h>

#include "mem.h"

typedef enum{FREE=0,ALLOCATED} State;
#define TAIL 1
typedef struct MetaData /*30 bits-size,1-state,1-flag to mark end of buffer*/
{
	
	size_t m_state:1;
	size_t m_tailFlag:1;
	size_t m_size:30;
	
		
}MetaData;

/*******************************Internal functions******************************/

/*receives address of block, if next block is free the function joins two blocks*/
void JoinBlocks(MetaData* _meta);

/*Splits block to two blocks in case it's size is bigger then needed,then allocates new block*/
void* SplitAndAlloc(MetaData *_metaData,size_t _newBlockSize);

/*Updates Metadata in given block*/
void UpdateBlock(MetaData* _block, size_t _size, State _state, int _tail);

/*******************************Internal layer**********************************/

void JoinBlocks(MetaData* _meta)
{
	MetaData* nextMeta=_meta+1+_meta->m_size/sizeof(MetaData);
	
	if(nextMeta->m_state!=FREE)
	{
		return;
	}	
	while(nextMeta->m_state==FREE && nextMeta->m_tailFlag!=TAIL)
	{
		_meta->m_size += nextMeta->m_size+sizeof(MetaData); 
		nextMeta+=1+nextMeta->m_size/sizeof(MetaData);	
		
	}			 
	return;	
}

void* SplitAndAlloc(MetaData *_metaData, size_t _newBlockSize)
{
	MetaData *next=NULL;	
	if(_metaData->m_size > (_newBlockSize+sizeof(MetaData)))
	{
		next=_metaData+1+_newBlockSize/sizeof(MetaData);
		UpdateBlock(next,_metaData->m_size - _newBlockSize-sizeof(MetaData),FREE,FREE);
	}
	
	_metaData->m_size=_newBlockSize;
	
	_metaData->m_state=ALLOCATED;/*change state to allocated*/
	
	return _metaData+1;
}


void UpdateBlock(MetaData* _block, size_t _size, State _state, int _tail)
{
	
	_block->m_size=_size;
	_block->m_size-=_block->m_size%sizeof(MetaData);
	_block->m_state=_state;
	_block->m_tailFlag=_tail;
}
/********************************************************************************/

/* _nBytes aligned to sizeof(int) */
void* MyMallocInit(void* _mem, size_t _nBytes)
{
	MetaData* metadata=NULL,*tail=NULL;
	
	
	assert(_mem);
	assert(_nBytes>sizeof(MetaData)*4);/*if nBytes are smaller than size of metadata*/
	
	metadata=_mem;
	
	/*Buffer metadata*/
	UpdateBlock(metadata,_nBytes-2*sizeof(MetaData),FREE,FREE);
	
	/*mark end of buffer*/	
	tail=metadata+metadata->m_size/sizeof(MetaData)+1;
	UpdateBlock(tail,0,ALLOCATED,TAIL);
	
	
	/*First Block's metadata*/
	++metadata;
	UpdateBlock(metadata,_nBytes-3*sizeof(MetaData),FREE,FREE);
		
	return _mem;
	
}


/* _nBytes will be aligned to sizeof(int) */
void* MyMalloc(void* _mem, size_t _nBytes)
{
	MetaData *metaData=NULL;
	size_t newBlockSize, aligment;
	
	assert(_mem);
	
	if(_nBytes==0)
	{
		return NULL;/*return NULL if asked size is 0*/
	}
	metaData=(MetaData*)_mem;
	newBlockSize=_nBytes;
	/*aligment do for _mem to*/
	aligment=newBlockSize%sizeof(int);
	if(aligment>0)
	{
		newBlockSize+=sizeof(int)-aligment;
	}
		
	
	if(newBlockSize>(metaData->m_size))/* return NULL if not enough contiguous memory to allocate*/
	{
		return NULL;
	}
	++metaData;/*first block's metadata*/
	
	/*search for empty block: while current block is not empty or don't have enough place*/
	while (metaData->m_state!=FREE || metaData->m_size < newBlockSize)
	{	
		if(metaData->m_tailFlag==TAIL)/*not found*/
		{
			return NULL;
		}
		if(metaData->m_state==FREE)
		{	
			JoinBlocks(metaData);
			{
				if(metaData->m_size >= newBlockSize)
				{
					return SplitAndAlloc(metaData,newBlockSize);
				}
			}	
		}
		metaData=metaData+1+metaData->m_size/sizeof(MetaData);
				
	}
	return SplitAndAlloc (metaData, newBlockSize);

}




void MyFree(void* _ptr)
{
	MetaData *metaData = NULL;
	
	assert(_ptr);
	
	metaData=(MetaData *)_ptr;
	--metaData;
	metaData->m_state=FREE;
		
	JoinBlocks(metaData);
	
}












