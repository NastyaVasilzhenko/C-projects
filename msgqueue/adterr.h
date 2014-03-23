/*Author:Anastasia Vasizlhenko
Date created:10.11.13
Date modified:18.12.13
Description: Library for the handle error function
*/
#ifndef _ADTERR_H_
#define _ADTERR_H_

typedef enum {
	ERR_OK=0,
	ERR_GENERAL,
	ERR_NOT_INITILIZED,
	ERR_UNDERFLOW,
	ERR_OVERFLOW,
	ERR_LOCKATION_FAILED,
	ERR_CANNOT_REDUCE_ORIGIN_SIZE,
	ERR_OUT_OF_RANGE,
	ERR_EMPTY_LIST
} ADTERR;

void HandleErr(ADTERR eErr,char* message);

#endif

