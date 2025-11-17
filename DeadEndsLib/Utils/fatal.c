// DeadEnds
//
// fatal.c holds the function _fatal that used to live in standard.c

#include <stdlib.h>

#include "config.h"

#include "standard.h"

// _fatal is the fatal error function. file and line are the file and line number of the call.
void _fatal (CString file, int line, CString msg, CString function)
// String file -- Name of file calling __fatal.
// int line -- Line number of file calling __fatal.
{
	printf ("FATAL ERROR!\n");
	if (msg)
		printf("%s, file: %s: line: %d, function: %s\n",
		       msg, file, line, function);
	else
		printf("file: %s: line: %d, function: %s\n",
		       file, line, function);
	abort();
}
