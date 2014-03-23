#include <stdio.h>
#include "verbose.h"

void verbPrintf(int verbosity, const char *msg)
{
    if (verbosity)
    {
    	printf ("%s\n",msg);
    }
}
