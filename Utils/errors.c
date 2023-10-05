//
//  DeadEnds
//
//  errors.c -- Code for handling DeadEnds errors.
//
//  Created by Thomas Wetmore on 4 July 2023.
//  Last changed on 7 July 2023.
//

#include "errors.h"
#include "list.h"

/*
  1. Errors when reading Gedcom files.  When these occur we can continue reading the file/s to
     get a list of further errors. But once all files have been read, the program must end with
     an admonishment to the user to fix errors before continuing. So there has to be an
     error-log for the user to use to see the list of errors.

     a. System errors.
        1. Failure to open a file.
        2. Failure to read a file.
        3. Memory allocation error.

     a. Errors in the lowest level syntax of the file. Eash of the errors in this category must
        provide the file name and the line number where the error occurs. There must be a way of
        recording multiple errors because we don't want the program to quit upon encountering the
        first error.

        1. Lines without level numbers.
        2. Lines without tags.
        3. 0 lines without keys.
        4. non-0 lines with keys.
        5. Keys with bad format.
        6. Lines with incorrect levels.

     b. Errors with Gedcom at a single record level.
        1. NAME lines without values.
        2. SEX lines without values or with incorrect values.
        3. FAMC, FAMS, HUSB, WIFE, CHIL lines without keys for values.

     c. Errors in the lineage linking.
        1. FAMC, FAMS, HUSB, WIFE and CHIL lines that don't point to other records. That is,
           each Gedcom files must be closed.
        2. Every FAMC, FAMS, HUSB, WIFE and CHIL link must have a return link from the
           INDI or FAM record it points to. That is, there must be consistent lineage-linking in
           every file.

   2. Errors when running DeadEnds programs. Run time errors can occur when running a DeadEnds
      program. I believe these are errors in another category that doesn't fit into the array
      handling notes above. When a run time error occurs the error is reported and execution of
      the program ends. The output of the error will have a file name and a line number associated
      with it, so the visual output of the error should be made similar to the format used for the
      errors encountered when reading Gedcom files and loading and validating them.

 WHAT IS THE OVERALL FLOW OF THE ERROR HANDLING?
   1. When trying to open files errors can occur. If an error occurs then, log it, set flag to
      not load a database, and continue.
   2. When reading a Gedcom files any errors should be logged to be reported on.
   3. When validating a Gedcom file any errors should be logged to be reported on.
   4. When all files have been read and validated, if there any errors they should be
      reported on then, and the program should not continue on to running DeadEnds programs.
 */

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
    stdfree(((Error*) error)->fileName);
    stdfree(((Error*) error)->message);
    return;
}

//  createErrorLog -- Create an error log.
//--------------------------------------------------------------------------------------------------
ErrorLog *createErrorLog(void)
{
    ErrorLog *errorLog = createList(compareError, deleteError,  getErrorKey);
    errorLog->keepSorted = true;
    return errorLog;
}

//  createError -- Create an error.
//--------------------------------------------------------------------------------------------------
Error *createError(ErrorType type, String fileName, int lineNumber, String message)
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
void addErrorToLog(ErrorLog *errorLog, ErrorType errorType, String fileName,
                   int lineNumber, String message)
{
    appendListElement(errorLog, createError(errorType, fileName, lineNumber, message));
}
