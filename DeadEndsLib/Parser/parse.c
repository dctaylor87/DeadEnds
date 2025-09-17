//
//  DeadEnds Library
//
//  parse.c contains two functions, parseProgram and parseFile, which parse DeadEnds scripts.
//
//  Created by Thomas Wetmore on 4 January 2023.
//  Last changed on 13 September 2025.
//

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include <stdarg.h>
#include <unistd.h>
#include "context.h"
#include "errors.h"
#include "functiontable.h"
#include "gedcom.h"
#include "hashtable.h"
#include "integertable.h"
#include "pnode.h"
#include "interp.h"
#include "list.h"
#include "parse.h"
#include "path.h"
#include "pvalue.h"
#include "set.h"
#include "stringset.h"
#include "symboltable.h"
//#include "refnindex.h"

static bool debugging = false;
static void parseFile(CString file, CString path, ErrorLog *errorLog); // Private function defined below.
static bool skipBOM (FILE *file, ErrorLog *errorLog, CString fileName); // Skip BOM if present

// Shared global variables. Memory ownership of the first four are taken over by a Program object.
SymbolTable* globals; // Global symbol table.
FunctionTable* functions; // User functions.
FunctionTable* procedures; // User procedures.
List* parsedFiles; // Parsed files (interned copies of file names).

List* pendingFiles; // Files to be parsed.
List* globalIdents; // Global identifiers.
CString curFileName = null; // File being parsed
FILE* currentFile = null; // FILE being parsed.
int curLine = 1; // Line number in current file.

// parseProgram parses a DeadEnds program and returns a Program object to be interpreted.
static void delete(void* a) { stdfree(a); }
Program* parseProgram(CString fileName, CString searchPath, ErrorLog *errorLog) {
    parsedFiles = createList(null, null, delete, false);  // Parsed file names.
    pendingFiles = createList(null, null, delete, false); // Queue of pending files.
	Set* included = createStringSet(); // Set of parsed file names.
    procedures = createFunctionTable(); // User-defined procedures.
    functions = createFunctionTable(); // User-defined functions.
    globalIdents = createList(null, null, delete, null); // Global identifiers.

    // Init pendingFiles with main program. Use strsave so parsedFiles can be deleted safely.
    enqueueList(pendingFiles, strsave(fileName));
    programParsing = true;

    // Parse the files in the pendingFiles queue.
    while (!isEmptyList(pendingFiles)) {
        String nextFile = (String) dequeueList(pendingFiles);
        if (!isInSet(included, nextFile)) {
            addToSet(included, nextFile);
            appendToList(parsedFiles, nextFile);
            parseFile(nextFile, searchPath, errorLog); // May add to queue.
        }
    }
    // Delete structures no longer needed.
    deleteList(pendingFiles);
    deleteSet(included);
    // If there were errors delete the other structures and return null.
    if (Perrors) {
        deleteList(parsedFiles);
        deleteList(globalIdents);
        deleteFunctionTable(procedures);
        deleteFunctionTable(functions);
        return null;
    }

    // Check that all called functions and procedures are defined.
    validateCalls(procedures, functions);


    // Parsing was successful. Create and return a Program.
    Program* program = createProgram();
    program->globalIdents = globalIdents;
    program->procedures = procedures;
    program->functions = functions;
    program->parsedFiles = parsedFiles;
    // Null the shared globals after the Program assumes ownership.
    procedures = functions = null;
    globalIdents = null;
    parsedFiles = null;
    programParsing = false;
    return program;
}

// parseFile parses a single script file with the yacc-generated parser.
static void parseFile(CString fileName, CString searchPath, ErrorLog *errorLog) {
    if (!fileName || *fileName == 0) return;
    curFileName = fileName;
    currentFile = fopenPath(fileName, "r", searchPath, "ll");
    if (!currentFile) {
      Error *error = createError (systemError, fileName, 0,
				  "file cannot be found\n");
      addErrorToLog (errorLog, error);
      
      curFileName = null;
      Perrors++;
      return;
    }
    if (! skipBOM (currentFile, errorLog, fileName))
      return;

    if (debugging) printf("Parsing %s.\n", fileName);
    curLine = 1;
    yyparse(errorLog); // Call the Yacc parser.
    fclose(currentFile);
}

static bool skipBOM (FILE *file, ErrorLog *errorLog, CString fileName)
{
  unsigned char bom[3];
  if (fread (bom, 1, 3, file) != 3)
    {
      Error *error = createError (systemError, fileName, 0,
				  "cannot read three bytes from file");
      addErrorToLog (errorLog, error);
      Perrors++;
      return false;
    }
  if ((bom[0] != 0xef) || (bom[1] != 0xbb) || (bom[2] != 0xbf))
    /* not bom, seek back */
    fseek (file, -3, SEEK_CUR);

  return true;
}
