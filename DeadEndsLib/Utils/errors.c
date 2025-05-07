//  DeadEnds
//
//  errors.c has code for handling DeadEnds errors.
//
//  Created by Thomas Wetmore on 4 July 2023.
//  Last changed on 5 May 2025.

#include "errors.h"
#include "list.h"

#define NUMKEYS 64
static bool debugging = false;

/* forward reference */
static void showErrorLog_1 (FILE *file, ErrorLog *errorLog);

// getKey returns the comparison key of an error.
static CString getKey(const void* error) {
	static char buffer[NUMKEYS][128];
	static int dex = 0;
	if (++dex > NUMKEYS - 1) dex = 0;
	String scratch = buffer[dex];
	CString fileName = ((Error*) error)->fileName;
	if (!fileName) fileName = "";
	int lineNumber = ((Error*) error)->lineNumber;
	sprintf(scratch, "%s%09d", fileName, lineNumber);
	return scratch; // Static memory!
}

// compare compares two errors for their placement in an error log.
static int compare(CString a, CString b) {
	return strcmp(a, b);
}

// delete frees an Error from an ErrorLog.
static void delete(void* error) {
	CString message = ((Error*) error)->message;
	if (message) stdfree(message);
	stdfree(error);
	return;
}

// createErrorLog creates an error log, a specialized List.
ErrorLog* createErrorLog(void) {
	ErrorLog* errorLog = createList(getKey, compare, delete, false);
	errorLog->sorted = true;
	return errorLog;
}

// createError creates an Error.
Error* createError(ErrorType type, CString fileName, int lineNumber, CString message) {
	Error *error = (Error*) stdalloc(sizeof(Error));
	if (! error)
	  return NULL;
	memset(error, 0, sizeof(Error));
	error->type = type;
	error->severity = severeError;
	error->fileName = strsave(fileName);
	error->lineNumber = lineNumber;
	error->message = strsave(message);
	if (debugging) printf("CREATE ERROR: %s, %d, %s\n", fileName, lineNumber, message);
	return error;
}

// setSeverityError changes the severity of an Error.
void setSeverityError(Error* error, ErrorSeverity severity) {
	error->severity = severity;
}

// deleteError deletes an Error.
void deleteError (Error* error) {
	if (error->message) stdfree(error->message);
	if (error->fileName) stdfree(error->fileName);
	stdfree(error);
}

// addErrorToLog adds an Error to an ErrorLog.
void addErrorToLog (ErrorLog* errorLog, Error* error) {
	if (!error) return;
	appendToList(errorLog, error);
}

// showError shows an Error on standard output.
void showError(FILE *file, Error* error) {
	fprintf(file, "error");
	if (error->fileName) fprintf(file, " in %s", error->fileName);
	if (error->lineNumber) fprintf(file, " line %d", error->lineNumber);
	if (error->message) fprintf(file, ": %s\n", error->message);
}

// deleteErrorLog deletes an ErrorLog.
void deleteErrorLog(ErrorLog* errorLog) {
	deleteList(errorLog);
}

// showErrorLog shows the contents of an ErrorLog on standard output.
void showErrorLog(ErrorLog* errorLog) {
  showErrorLog_1 (stdout, errorLog);
}

static void showErrorLog_1 (FILE *file, ErrorLog *errorLog) {
	if (!errorLog) {
		fprintf(file, "Error log does not exist.\n");
		return;
	}
	if (lengthList(errorLog) == 0) {
		fprintf(file, "Error log is empty.\n");
		return;
	}
	fprintf(file, "Error log:\n");
	sortList(errorLog);
	FORLIST(errorLog, error)
		showError(file, (Error*) error);
	ENDLIST
}

bool saveErrorLog (String filename, ErrorLog *errorLog)
{
  FILE *file = fopen (filename, "w");

  if (! file)
    return false;

  showErrorLog_1 (file, errorLog);
  fclose (file);
  return true;
}
