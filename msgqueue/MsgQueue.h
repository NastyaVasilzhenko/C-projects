/**************************************************************************************
    Author :				
    Creation date :      	10.1.12
    Date last modified :
    Description : 			Message Queue over Shared Memory in multi-process interlock
***************************************************************************************/
#ifndef __MSG_QUEUE_H_
#define __MSG_QUEUE_H_

typedef struct MQ MQ_t;

/* creates and initializes a new message queue object.received size is a netto size*/
MQ_t*	MsgQueueCreate	(char *_msgQueueName, size_t _msgMax, size_t _memSize); 

/*connects to already existing message queue object*/
MQ_t*	MsgQueueConnect	(char *_msgQueueName);

/* disconnects from message queue and destroys the shared objects(if this process is a creator)*/
void	MsgQueueDestroy	(MQ_t *_msqQue);

/* return number of bytes actually written, 0 if message queue full		*/
int		MsgQueueSend	(MQ_t *_msgQue, void* _buffer, size_t _length);

/* return number of bytes actually read, 0 if message queue unavailable */ 
int		MsgQueueRecv	(MQ_t *_msgQue, void* _buffer, size_t _size);

int		IsMsgQueueEmpty (MQ_t *_msgQue);


#endif /* __MSG_QUEUE_H_ */
