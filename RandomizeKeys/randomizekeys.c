// DeadEnds Tool
//
// randomizekeys.c is the DeadEnds tool that randomizes the keys in a Gedcom file. It reads a
// Gedcom file, generates a new random key for every record in the file, and then rewrites the
// randomized Gedcom file to standard output.
//
// Created by Thomas Wetmore on 14 July 2024.
// Last changed on 25 November 2024.

#include <stdint.h>

#include "randomizekeys.h"

static void getArguments(int, char**, CString*);
static void getEnvironment(CString*);
static void usage(void);
static void goAway(ErrorLog*);

static bool debugging = true;

// main is the main program of the randomize keys batch program.
int main(int argc, char** argv) {
	CString gedcomFile = null;
	CString searchPath = null;
	printf("%s: RandomizeKeys begin.\n", getMillisecondsString());
	getArguments(argc, argv, &gedcomFile);
	getEnvironment(&searchPath);
	gedcomFile = resolveFile(gedcomFile, searchPath);
	if (debugging) printf("Resolved file: %s\n", gedcomFile);
	// Get the Gedcom records from a file.
	File* file = openFile(gedcomFile, "r");
	ErrorLog* log = createErrorLog();

	// Parse the Gedcom file and build a GNodeList of its records.
	IntegerTable* keymap = createIntegerTable(4097);
	GNodeList* roots = getGNodeTreesFromFile(file, keymap, log);
	printf("ramdomize keys: %s: read gedcom file.\n", getMillisecondsString());
	if (lengthList(log) > 0) goAway(log);
	closeFile(file);

	// Validate the keys.
	checkKeysAndReferences(roots, file->name, keymap, log);
	printf("ramdomize keys: %s: validated keys.\n", getMillisecondsString());
	if (lengthList(log)) {
		deleteGNodeList(roots, basicDelete);
		goAway(log);
	}

	// Create a table to map existing keys to random keys.
	StringTable* keyTable = createStringTable(1025);
	initRecordKeyGenerator();
	FORLIST(roots, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		String key = root->key;
		if (!key) continue;
		RecordType r = recordType(root);
		String newKey = generateRecordKey(r);
		addToStringTable(keyTable, key, newKey);
	ENDLIST
	printf("ramdomize keys: %s: created remap table.\n", getMillisecondsString());

	// Change the keys throughout the list.
	FORLIST(roots, element)
		// Change the key on the root.
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		String key = root->key;
		if (key)  {
			String new = searchStringTable(keyTable, key);
			stdfree(root->key);
			root->key = new;
		}
		// Change all values that are keys.
		FORTRAVERSE(root, node)
			if (isKey(node->value)) {
				String new = searchStringTable(keyTable, node->value);
				stdfree(node->value);
				node->value = strsave(new);
			}
		ENDTRAVERSE
	ENDLIST
	printf("ramdomize keys: %s: rekeyed records.\n", getMillisecondsString());

	// Write the modified GNodeList to standard out.
	FORLIST(roots, element)
		GNodeListEl* el = (GNodeListEl*) element;
		writeGNodeRecord(stdout, el->node, false);
	ENDLIST
	printf("ramdomize keys: %s: wrote gedcom file.\n", getMillisecondsString());
	return 0;
}

// getFileArguments gets the file names from the command line. They are mandatory.
static void getArguments(int argc, char* argv[], CString* gedcomFile) {
	int ch;
	while ((ch = getopt(argc, argv, "g:")) != -1) {
		switch(ch) {
		case 'g':
			*gedcomFile = strsave(optarg);
			break;
		case '?':
		default:
			usage();
			exit(1);
		}
	}
	if (!*gedcomFile) {
		usage();
		exit(1);
	}
}

// getEnvironment checks for the DE_GEDCOM_PATH env variable.
static void getEnvironment(CString* gedcomPath) {
	*gedcomPath = getenv("DE_GEDCOM_PATH");
	if (!*gedcomPath) *gedcomPath = ".";
}

// usage prints the RunScript usage message.
static void usage(void) {
	fprintf(stderr, "usage: RandomizeKeys -g gedcomfile\n");
}

// goAway is called if anything does wrong. It prints the error log and exits.
static void goAway(ErrorLog* log) {
	printf("randomizekeys: cancelled due to errors\n");
	showErrorLog(log);;
	exit(1);
}

