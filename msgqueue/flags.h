/**************************
Author:Anastasia Vasilzhenko
Date created: 20.2.14
Date modified:24.2.14
Description:  Flags.
-v verbosity -default 0
-s sleep time - default 2
-m message queue name (messages channel) -default "flags.h"
-h message queue name (hand shake channel) -default "flags.c"
-n num of send messages (Ping) -default 24
***************************************************************************************/

#ifndef _FLAGS_H_
#define _FLAGS_H_


#define NUM_TO_MILLI 1000
#define LEN_FILE 100

typedef struct Params
{
	int m_sleep;
	char m_fileNameMSG[LEN_FILE];
	char m_fileNameHS[LEN_FILE];
	int m_verbose;
	size_t m_numOfMsg;
	
} Params;

/*Receives input from user,sets default*/
int RetrieveFlags(Params *_params, int _argc, char **_argv); 

#endif
