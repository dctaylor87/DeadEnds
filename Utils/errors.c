//
//  DeadEnds
//
//  errors.c -- Code for handling DeadEnds errors.
//
//  Created by Thomas Wetmore on 4 July 2023.
//  Last changed on 5 November 2023.
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
static String getErrorKey(Word error)
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

//  compareError -- Compare two errors for their placement in an error log.
//--------------------------------------------------------------------------------------------------
static int compareError(Word errorOne, Word errorTwo)
{
	String key1 = getErrorKey(errorOne);
	String key2 = getErrorKey(errorTwo);
	return strcmp(key1, key2);
}

//  deleteError -- Free up the memory for an error when an error log is freed.
//--------------------------------------------------------------------------------------------------
static void deleteError(Word error)
{
	String fileName = ((Error*) error)->fileName;
	String message = ((Error*) error)->message;
	if (fileName) stdfree(fileName);
	if (message) stdfree(message);
	stdfree(error);
	return;
}

//  createErrorLog -- Create an error log. An ErrorLog is a specialized List.
//--------------------------------------------------------------------------------------------------
ErrorLog *createErrorLog(void)
{
	ErrorLog *errorLog = createList(compareError, deleteError,  getErrorKey);
	errorLog->keepSorted = true;
	return errorLog;
}

//  createError -- Create an error.
//--------------------------------------------------------------------------------------------------
Error *createError(ErrorType type, CString fileName, int lineNumber, String message)
{
	Error *error = (Error*) stdalloc(sizeof(Error));
	error->type = type;
	error->fileName = strsave(fileName);
	error->lineNumber = lineNumber;
	error->message = strsave(message);
	return error;
}

//  addErrorToLog -- Add an error to an error log.
//--------------------------------------------------------------------------------------------------
void addErrorToLog(ErrorLog *errorLog, ErrorType errorType, CString fileName, int lineNumber,
	String message)
{
	appendListElement(errorLog, createError(errorType, fileName, lineNumber, message));
}
