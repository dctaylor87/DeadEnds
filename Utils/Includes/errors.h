//
//  errors.h
//  JustParsing
//
//  Created by Thomas Wetmore on 4 July 2023.
//  Last changed on 5 July 2023.
//

#ifndef errors_h
#define errors_h

#include "standard.h"
#include "list.h"

//  ErrorType -- Types of errors.
//--------------------------------------------------------------------------------------------------
typedef enum {
    systemError,
    syntaxError,
    gedcomError,
    linkageError
} ErrorType;

// Error -- structure for holding an error.
//--------------------------------------------------------------------------------------------------

typedef struct {
    ErrorType type;  //  Type of this error.
    String fileName;  //  Name of file, if any, containing the error.
    int lineNumber;  //  Line number in file, if any, where the error occurs.
    String message;  //  Message that describes the error.
} Error;

//  API to error logs.
//--------------------------------------------------------------------------------------------------
#define ErrorLog List

extern ErrorLog *createErrorLog(void);
extern Error *createError(ErrorType type, String fileName, int lineNumber, String message);
extern void addErrorToLog(ErrorLog* log, ErrorType, String, int, String);

#endif // errors_h
