/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified:23.2.14
Description:  Test program: Connects to two shared memory objects, receives messages.
***************************************************************************************/

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "MsgQueue.h"
#include "verbose.h"
#include "flags.h"

/*global*/
Params g_params;
/***************************************/
#define	MESSAGES_MSGQ_NAME	"Ping.c"
#define	HS_MSGQ_NAME 		"Pong.c"


#define MAX_MSG_SIZE	32
#define	HANDSHAKE		"Hello"
#define	BYE_BYE			"Bye Bye"
/**************************************/

int main(int argc, char **argv)
{
	
	MQ_t 		*msgQue = NULL, *registerMQ_t = NULL;
	char		msg[MAX_MSG_SIZE];
	size_t		size;

	verbPrintf(g_params.m_verbose, "Ping: retrive flags");
	if(RetrieveFlags(&g_params, argc, argv)== -1)
	{
		perror("ERROR:Unknown option");
        return 1;
	}

	/* Connect to Message Queues */
	registerMQ_t =MsgQueueConnect(HS_MSGQ_NAME);
	msgQue  =MsgQueueConnect(MESSAGES_MSGQ_NAME);

	verbPrintf(g_params.m_verbose, "Pong: register new pong");
	MsgQueueSend(registerMQ_t,HANDSHAKE,strlen(msg));
	
	while (1)
	{
		/* Read Message */
		verbPrintf(g_params.m_verbose, "Pong: get message");
		size = MsgQueueRecv(msgQue,msg,MAX_MSG_SIZE);
		msg[size] = 0;
		printf("Receive Message:%s\n",msg);

		/* Work */
		usleep(g_params.m_sleep);
		
		/* Check if ping is down */
		if (!strcmp(BYE_BYE,msg))
		{
			verbPrintf(g_params.m_verbose, "Pong: got eof message");
			break;
		}			
	} 
	verbPrintf(g_params.m_verbose, "Pong: send eof message back");
	MsgQueueSend(msgQue,msg,size);
	
	/*Destroy*/
	MsgQueueDestroy(registerMQ_t);
	MsgQueueDestroy(msgQue);
	
	return 0;
}



