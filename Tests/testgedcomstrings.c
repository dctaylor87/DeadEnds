// DeadEnds
//
// testgedcomstrings.c
//
// Created by Thomas Wetmore on 27 May 2024.
// Last changed on 28 May 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "standard.h"
#include "readnode.h"

char* readFileIntoString(const char *filename);

// testGedcomStrings tests the stringToGnodeTree and showGNodeTree functions.
void testGedcomStrings(void) {
	printf("START OF TEST GEDCOM STRINGS\n");
	String file = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/ttw.ged";
	String record = readFileIntoString(file);
	printf("The length of the record is %ld\n", strlen(record));
	GNode* root = stringToGNodeTree(record, null);
	showGNodeTree(root);
	printf("END OF TEST GEDCOM STRINGS\n");
}

// readFileIntoString reads a file into a string; written by ChatGPT.
char* readFileIntoString(const char* filename) {
	FILE* file = fopen(filename, "r");
	char* buffer = NULL;
	long length;

	if (file) {
		fseek(file, 0, SEEK_END); // Get length.
		length = ftell(file);
		fseek(file, 0, SEEK_SET);
		buffer = (char*)malloc((length + 1) * sizeof(char));
		if (buffer) {
			fread(buffer, 1, length, file);
			buffer[length] = '\0';
		}
		fclose(file);
	} else {
		fprintf(stderr, "Could not open file %s for reading\n", filename);
	}
	return buffer;
}
