/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified:23.2.14
Description:  Test program: Creates two shared memory objects, sends messages.
***************************************************************************************/

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "flags.h"
#include "verbose.h"
#include "Sync.h"
#include "MsgQueue.h"

/*global*/
Params g_params;

/*****Threads arguments**********/
typedef struct ReadParams
{
	MQ_t*				m_msgQ;
	sem_t*				m_firstPong;
	pthread_mutex_t*	m_mutex;
	size_t*				m_msgCounter;
	size_t*				m_numOfMsg;
}ReadParams;

typedef struct WriteParams
{
	MQ_t*				m_msgQ;
	sem_t*				m_firstPong;
	pthread_mutex_t*	m_mutex;
	size_t*				m_msgCounter;

}WriteParams;

/**************Default values********************/

#define	HS_MSGQ_LENGTH	10
#define	HS_MSGQ_MEM_SIZE	1024

#define	MESSAGES_MSGQ_LENGTH	10
#define	MESSAGES_MSGQ_MEM_SIZE	1024


#define	MESSAGES_MSGQ_NAME	"Ping.c"
#define	HS_MSGQ_NAME 		"Pong.c"

#define MAX_MSG_SIZE	32
#define	HANDSHAKE		"Hello"
#define	BYE_BYE			"Bye Bye"

/************Thread Functions*******************/
static void* Read (void* _params);
static void* Write (void* _params);

/**********************************************/

int main(int argc, char **argv)
{

	pthread_t		read, write;
	ReadParams		readParams;
	WriteParams		writeParams;			
	sem_t			firstPong;
	pthread_mutex_t	mutex;
	size_t			counter;
	
	
	verbPrintf(g_params.m_verbose, "Ping: retrive flags");
	if(RetrieveFlags(&g_params, argc, argv)== -1)
	{
		perror("ERROR:Unknown option");
        	return 1;
	}
	
	verbPrintf(g_params.m_verbose, "Ping: sync tools init");
	if(sem_init(&firstPong, 0, 0)==-1)
	{
		perror("ERROR:sem init");
        return 1;
	}
	
	if(pthread_mutex_init(&mutex,0)==-1)
	{
		perror("ERROR:mutex init");
        return 1;
	}
	counter = 0;

	verbPrintf(g_params.m_verbose, "Ping: create handshake channel");
	readParams.m_msgQ		=  MsgQueueCreate(HS_MSGQ_NAME,HS_MSGQ_LENGTH,HS_MSGQ_MEM_SIZE);
	readParams.m_firstPong  =  &firstPong;
	readParams.m_mutex 		=  &mutex;
	readParams.m_msgCounter =  &counter;
	readParams.m_numOfMsg 	=  &g_params.m_numOfMsg;

	verbPrintf(g_params.m_verbose, "Ping: create handshake thread");
	pthread_create(&read,  NULL, Read, (void*)&readParams);

	writeParams.m_msgQ			 = MsgQueueCreate(MESSAGES_MSGQ_NAME,MESSAGES_MSGQ_LENGTH,MESSAGES_MSGQ_MEM_SIZE);
	writeParams.m_firstPong	     = &firstPong;
	writeParams.m_mutex			 = &mutex;
	writeParams.m_msgCounter	 = &counter;
	
	verbPrintf(g_params.m_verbose, "Ping: create msg thread");
	pthread_create(&write, NULL, Write, (void*)&writeParams);

	pthread_join(write,0);

	/* Destroy */
	MsgQueueDestroy(writeParams.m_msgQ);
	MsgQueueDestroy(readParams.m_msgQ);	
	sem_destroy(&firstPong);
	pthread_mutex_destroy(&mutex);

	return 0;
}

/**************************************************************************************/

static void* 	Read(void* _params)
{
	ReadParams*	 params = (ReadParams*)_params;
	char		 msg[MAX_MSG_SIZE];
	size_t		 size;
	int			 isFirstFlag = 1;
	
	verbPrintf(g_params.m_verbose, "Ping:READ");
	
	while (1)
	{
		verbPrintf(g_params.m_verbose, "Read: Wait for handshake");
		size = MsgQueueRecv(params->m_msgQ,msg,MAX_MSG_SIZE);
		msg[size] = '\0';
			
		/* Recieved Pong up msg  */
		if (!strcmp(msg,HANDSHAKE))
		{
			verbPrintf(g_params.m_verbose, "Read: Handshake received");
			pthread_mutex_lock(params->m_mutex);
			*(params->m_msgCounter) += *(params->m_numOfMsg);
			pthread_mutex_unlock(params->m_mutex);

			/* Notify Write - first pong*/		
			if (isFirstFlag)
			{
				if (sem_post (params->m_firstPong) == -1)
				{
					perror ("Read:ERROR: Sem Post Failed");
					return (void*)1;
				}
				verbPrintf(g_params.m_verbose, "Read: Num of msgs increased");
				isFirstFlag = 0;
			}
		}
	} 
	return 0;	
}

/**************************************************************************************/

static void* 	Write(void* _params)
{
	WriteParams* params = (WriteParams*)_params;
	char		 msg[MAX_MSG_SIZE];
	int			 counter=1;
	pid_t		 pid= getpid();
	
	verbPrintf(g_params.m_verbose, "Ping:WRITE");

	/* Wait for first pong */
	if (sem_wait(params->m_firstPong) == -1)
	{
		perror ("Write:ERROR:Sem Wait Failed");
		return (void*)1;
	}

	verbPrintf(g_params.m_verbose, "Write: Woke up");

	while(1)
	{
		pthread_mutex_lock(params->m_mutex);
		if(*(params->m_msgCounter)<1)
		{
			pthread_mutex_unlock(params->m_mutex);
			break;
		}
		pthread_mutex_unlock(params->m_mutex);
		
		verbPrintf(g_params.m_verbose, "Write: Send Msg");
		sprintf(msg,"process num:%d msg num: %d",pid, counter);
		MsgQueueSend(params->m_msgQ,msg,strlen(msg));
		++counter;
		
		verbPrintf(g_params.m_verbose, "Write: Update msg counter");
		pthread_mutex_lock(params->m_mutex);
		--*(params->m_msgCounter);
		pthread_mutex_unlock(params->m_mutex);
		
		usleep(g_params.m_sleep);
	}

	sprintf(msg,BYE_BYE);
	MsgQueueSend(params->m_msgQ,msg,strlen(msg));
	verbPrintf(g_params.m_verbose, "Write: Ping is done");
	return 0;
	
}
