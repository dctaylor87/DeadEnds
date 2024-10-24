// DeadEnds
//
// importone.c has the test function that tries to create a Database for the rest of the tests.
//
// Created by Thomas Wetmore on 21 June 2024.
// Last changed on 15 October 2024.

#include "standard.h"
#include "import.h"
#include "utils.h"

// importDatabaseTest tests the new import organization.
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
