// DeadEnds
//
// importone.c has the test function that tries to create a Database for the rest of the tests.
//
// Created by Thomas Wetmore on 21 June 2024.
// Last changed on 22 July 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "standard.h"
#include "errors.h"

#include "file.h"
#include "import.h"
#include "utils.h"

// importTest tests the new import organization.
 Database* importDatabaseTest(ErrorLog* log, int testNumber) {
	printf("%d: START OF IMPORT DATABASE TEST: %s %s\n", testNumber, "modified.ged", getMillisecondsString());
	String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/modified.ged";
	String lastSegment = lastPathSegment(gedcomFile);
	printf("lastPathSegment: %s\n", lastSegment);
	Database* database = gedcomFileToDatabase(gedcomFile, log);
	if (lengthList(log)) {
		printf("Import cancelled because of errors:\n");
		showErrorLog(log);
	}
	summarizeDatabase(database);
	printf("%d: END OF CREATE DATABASE TEST: %s\n", testNumber, getMillisecondsString());
	return database;
}