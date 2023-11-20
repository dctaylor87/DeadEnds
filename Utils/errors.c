//
//  DeadEnds
//
//  errors.c -- Code for handling DeadEnds errors.
//
//  Created by Thomas Wetmore on 4 July 2023.
//  Last changed on 20 November 2023.
//

#include "errors.h"
#include "list.h"

//  getErrorKey -- Get the comparison key of an error.
//
//    This needs a little thought. We need to sort the errors by a key made up of the file name
//    and the line number. But The sorting has to be done numerically with the line numbers and
//    not lexicographably.
//--------------------------------------------------------------------------------------------------
#define NUMKEYS 64
static String getErrKey(Word error)
{
	static char buffer[NUMKEYS][128];
	static int dex = 0;
	if (++dex > NUMKEYS - 1) dex = 0;
	String scratch = buffer[dex];
	String fileName = ((Error*) error)->fileName;
	if (!fileName) fileName = "";
	int lineNumber = ((Error*) error)->lineNumber;
	//  MNOTE: The key is returned out of static memory. The caller must make a copy if it needs
	//  to persist beyond NUMKEYs calls of this function.
	sprintf(scratch, "%s%09d", fileName, lineNumber);
	return scratch;
}

//  cmpError -- Compare two errors for their placement in an error log.
//--------------------------------------------------------------------------------------------------
static int cmpError(Word errorOne, Word errorTwo)
{
	String key1 = getErrKey(errorOne);
	String key2 = getErrKey(errorTwo);
	return strcmp(key1, key2);
}

//  delError -- Free up the memory for an error when an error log is freed.
//--------------------------------------------------------------------------------------------------
static void delError(Word error)
{
	String message = ((Error*) error)->message;
	if (message) stdfree(message);
	stdfree(error);
	return;
}

//  createErrorLog -- Create an error log. An ErrorLog is a specialized List.
//--------------------------------------------------------------------------------------------------
ErrorLog *createErrorLog(void)
{
	ErrorLog *errorLog = createList(cmpError, delError,  getErrKey);
	errorLog->keepSorted = true;
	return errorLog;
}

//  createError -- Create an Error.
//--------------------------------------------------------------------------------------------------
Error *createError(ErrorType type, String fileName, int lineNumber, String message)
{
	Error *error = (Error*) stdalloc(sizeof(Error));
	error->type = type;
	error->fileName = fileName;  // MNOTE: Not saved; do not free.
	error->lineNumber = lineNumber;
	error->message = strsave(message);
	return error;
}

//  deleteError -- Delete an Error. MNOTE: Not freeing fileName.
//-------------------------------------------------------------------------------------------------
void deleteError (Error *error)
{
	if (error->message) stdfree(error->message);
	stdfree(error);
}

//  addErrorToLog -- Add an Error to an ErrorLog.
//-------------------------------------------------------------------------------------------------
void addErrorToLog (ErrorLog *errorLog, Error *error)
{
	appendListElement(errorLog, error);
}

//  addErrorToLog -- Add an error to an error log.
//--------------------------------------------------------------------------------------------------
void oldAddErrorToLog(ErrorLog *errorLog, ErrorType errorType, String fileName, int lineNumber,
	String message)
{
	appendListElement(errorLog, createError(errorType, fileName, lineNumber, message));
}
