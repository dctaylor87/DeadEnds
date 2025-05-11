//
//  DeadEnds
//
//  functiontable.c -- The DeadEnds function and procedure table datatype.
//
//  Created by Thomas Wetmore on 9 April 2023.
//  Last changed on 10 May 2025.

#include <stdint.h>

#include "refnindex.h"
#include "functiontable.h"

static bool debugging = false;

// compare is the compare function for FunctionTables.
static int compare(CString a, CString b) {
	return strcmp(a, b);
}

// getKey is the getKey function for FunctionTables.
static CString getKey(void* element)
{
	FunctionElement* funcEl = (FunctionElement*) element;
	return funcEl->name;
}

// delete is the delete function for FunctionTables.
static void delete(void* element) {
	FunctionElement* funcEl = (FunctionElement*) element;
	stdfree(funcEl->name);
	freePNodes(funcEl->function);
}

// createFunctionTable creates a FunctionTable.
static int numBucketsInFunctionTable = 13;
FunctionTable* createFunctionTable(void) {
	return createHashTable(getKey, compare, delete, numBucketsInFunctionTable);
}

// searchFunctionTable searches a function table for the function or procedure with given name.
PNode* searchFunctionTable(FunctionTable* table, String name) {
	FunctionElement* element = (FunctionElement*) searchHashTable(table, name);
	return element ? element->function : null;
}

// addToFunctionTable adds a user defined function or procedure to a function table. Takes the
// name and the root PNode as parameters.
void addToFunctionTable(FunctionTable* table, String name, PNode* pnode) {
	if (debugging) printf("Adding %s to a function table.\n", name);
	FunctionElement* element = (FunctionElement*) searchHashTable(table, name);
	if (element) {
		printf("Multiply defined function or procedure named %s; new copied over old.\n", name);
	}
	element = (FunctionElement*) stdalloc(sizeof(FunctionElement)); // Create element.
	memset(element, 0, sizeof(FunctionElement));
	element->name = strsave(name);
	element->function = pnode;
	addToHashTable(table, element, true);
}
