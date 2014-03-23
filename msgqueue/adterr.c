/*Author:Anastasia Vasizlhenko
Date created:10.11.13
Date modified:18.12.13
Description: Prints type of an error and message with discriptions
*/

#include <stdio.h>
#include "adterr.h"

void HandleErr(ADTERR eErr,char* message)
{
	char* error[]={
		"Success","General error",
		"error-not initialized",
		"error overflow",
		"error underflow",
		"location failed",
		"error-imposible to reduce origin size",
		"error-out of range",
		"List is empty"
		};
	if (eErr)
		printf("ErrDescription=%s Message= %s\n", error[eErr], message); 
}
