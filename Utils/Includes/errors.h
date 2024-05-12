//
//  errors.h
//  DeadEnds
//
//  Created by Thomas Wetmore on 4 July 2023.
//  Last changed on 25 November 2023.
//

#ifndef errors_h
#define errors_h

#include "standard.h"
#include "list.h"

//  ErrorType -- Types of errors.
//--------------------------------------------------------------------------------------------------
typedef enum ErrorType {
	systemError,
	syntaxError,
	gedcomError,
	linkageError
} ErrorType;

typedef enum ErrorSeverity {
	fatalError,    // Quit loading current database immediately,
	severeError,   // Continue with this file but don't keep database,
	warningError,  // Continue with this file and load database.
	commentError   // Message for user -- not an error.
} ErrorSeverity;

// Error -- structure for holding an error.
//--------------------------------------------------------------------------------------------------
typedef struct Error {
	ErrorType type;         //  Type of this error.
	ErrorSeverity severity; //  Severity of this error.
	CString fileName;        //  Name of file, if any, containing the error.
	int lineNumber;         //  Line number in file, if any, where the error occurs.
	String message;         //  Message that describes the error.
} Error;

//  API to error logs.
//--------------------------------------------------------------------------------------------------
#define ErrorLog List

extern ErrorLog *createErrorLog(void);
extern void deleteErrorLog(ErrorLog*);
extern Error *createError(ErrorType type, CString fileName, int lineNumber, String message);
extern void setSeverityError(Error *error, ErrorSeverity severity);
extern void deleteError(Error*);
extern void addErrorToLog(ErrorLog*, Error*);
extern void oldAddErrorToLog(ErrorLog *errorLog, ErrorType errorType,
			     CString fileName, int lineNumber, String message);
extern void showErrorLog(ErrorLog*);
extern void showError(Error*);

#endif // errors_h
