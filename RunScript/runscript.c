// DeadEnds
//
// main.c is the DeadEnds RunScript program. It has three steps.
//  1. Create a Database from a Gedcom file.
//  2. Parse a DeadEnds script file into its internal form.
//  3. Run the script on the Database and write any output to stdout.
//
// usage: runscript -g gedcomfile -s scriptfile
//
// If DE_GEDCOM_PATH and/or DE_SCRIPTS_PATH are defined, they may be used to find the files.
//
// Created by Thomas Wetmore on 21 July 2024
// Last changed on 7 December 2024.

#include <stdint.h>

#include "runscript.h"
#include "interp.h"
#include "parse.h"

// Local functions.
static void usage(void);
static void getArguments(int, char**, CString*, CString*);
static void getEnvironment(CString*, CString*);
static void runScript(Database*, CString);

// main is the main program of the RunScript program.
int main(int argc, char* argv[]) {
	// Get the files.
	fprintf(stderr, "%s: RunScript started.\n", getMsecondsStr());
	CString gedcomFile = null;
	CString scriptFile = null;
	CString gedcomPath = null;
	CString scriptPath = null;
	getArguments(argc, argv, &gedcomFile, &scriptFile);
	getEnvironment(&gedcomPath, &scriptPath);
	// Build the Database from the Gedcom file.
		gedcomFile = resolveFile(gedcomFile, gedcomPath);
		ErrorLog* errorLog = createErrorLog();
		Database* database = getDatabaseFromFile(gedcomFile, 0, errorLog);
		if (lengthList(errorLog)) {
			showErrorLog(errorLog);
			exit(1);
		}
		fprintf(stderr, "%s: Database created.\n", getMsecondsStr());
	// Parse and run the script.
	parseProgram(scriptFile, scriptPath);
	fprintf(stderr, "%s: Script parsed.\n", getMsecondsStr());
	runScript(database, scriptFile);
	fprintf(stderr, "%s: RunScript done.\n", getMsecondsStr());
}

// getArguments gets the file names from the command line.
void getArguments(int argc, char* argv[], CString* gedcom, CString* script) {
	int ch;
	while ((ch = getopt(argc, argv, "g:s:")) != -1) {
		switch(ch) {
		case 'g':
			*gedcom = strsave(optarg);
			break;
		case 's':
			*script = strsave(optarg);
			break;
		case '?':
		default:
			usage();
			exit(1);
		}
	}
	if (!*gedcom || !*script) {
		usage();
		exit(1);
	}
}

// getEnvironment looks for DE_GEDCOM_PATH or DE_SCRIPTS_PATH in the environment.
static void getEnvironment(CString* gedcom, CString* script) {
	*gedcom = getenv("DE_GEDCOM_PATH");
	*script = getenv("DE_SCRIPTS_PATH");
	if (!*gedcom) *gedcom = ".";
	if (!*script) *script = ".";
}

// runScript runs a script by interpreting its main proc. After a script is parsed there must
// be a proc named "main" in the global procedureTable. runScript creates a PNode to call that
// proc, then creates the Context to call it in, and then calls it. The Context consists of the
// Database and the main proc's SymbolTable.
static void runScript(Database* database, CString fileName) {
	// Create a PNode to call the main proc.
	curFileName = "internal";
	curLine = 1;
	PNode* pnode = procCallPNode("main", null);
	// Call the main proc.
	SymbolTable* symbols = createSymbolTable();
	Context* context = createContext(symbols, database);
	interpret(pnode, context, null); // Call main proc.
}

// usage prints the RunScript usage message.
static void usage(void) {
	fprintf(stderr, "usage: runscript -g gedcomfile -s scriptfile\n");
}
