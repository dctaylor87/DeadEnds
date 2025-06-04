//
//  DeadEnds Library
//
//  symboltable.h is the header file for the symbol tables that hold the values of variables in
//  DeadEnds programs. Symbol tables are implented with hash tables.
//
//  Created by Thomas Wetmore on 23 March 2023.
//  Last changed on 3 June 2025.
//

#ifndef symboltable_h
#define symboltable_h

#include "standard.h"

typedef struct PValue PValue;
typedef struct PNode PNode;
typedef struct Frame Frame;
typedef struct Context Context;
typedef struct HashTable HashTable;

// A SymbolTable holds DeadEnds script variables and their Pvalues.
typedef HashTable SymbolTable;

// A Symbol is an element in a SymbolTable; ident is the name of a variable/identifier, and value is its PValue.
typedef struct Symbol {
	CString ident;
	PValue *value;
} Symbol;

extern bool symbolTableDebugging;

//  Interface to SymbolTable.
SymbolTable *createSymbolTable(void);
void deleteSymbolTable(SymbolTable*);
void assignValueToSymbol(Context*, CString, PValue);
void assignValueToSymbolTable(SymbolTable*, CString, PValue);
PValue getValueOfSymbol(Context*, CString);
PValue getValueFromSymbolTable(SymbolTable*, CString);
void showSymbolTable(SymbolTable*); // Debug.

#endif // symboltable_h
