/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified:24.2.14
Description:  Flags Retrive. Get options from user-if not,set defaults
-v verbosity - default 0
-s sleep time - default 2
-m message queue name (messages channel) -default "flags.h"
-h message queue name (hand shake channel) -default "flags.c"
-n num of send messages (Ping) -default 24
****************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>  
#include <sys/types.h>
#include <getopt.h>

#include "flags.h"

#define SLEEP_DEFAULT 2
#define VERB_DEFAULT 0
#define MSG_NAME "flags.h"
#define HANDSHAKE_NAME "flags.c"
#define NUM_OF_MSG 24

int RetrieveFlags(Params *_params, int _argc, char **_argv)
{
	int opts; 
	
	/*default*/
	
	_params->m_verbose = VERB_DEFAULT;
	_params->m_sleep = SLEEP_DEFAULT*NUM_TO_MILLI;
	_params->m_numOfMsg=NUM_OF_MSG;
	strcpy(_params->m_fileNameMSG, MSG_NAME);
	strcpy(_params->m_fileNameHS, HANDSHAKE_NAME);

	/*INPUT FROM USER*/	

	while((opts = getopt(_argc, _argv, "vs:m:h:n:")) != -1)
	{
		switch (opts)
		{
			case 'v':
		
				_params->m_verbose = 1;
				break;
			
			case 'n':
		
				_params->m_numOfMsg = atoi(optarg);
				break;		
		
			case 's':
			
				_params->m_sleep = atoi(optarg)*NUM_TO_MILLI;
				break;
	
				
			case 'm':
			
				strcpy(_params->m_fileNameMSG, optarg);
				break;
		
			case 'h':
			
				strcpy(_params->m_fileNameHS, optarg);
				break;
				
			default:
			
				return -1;
				break;
		}
	}
	return 0;
}

