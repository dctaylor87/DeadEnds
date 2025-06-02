//
// DeadEnds
//
// symboltable.h -- Header file for the symbol tables that hold the values of variables in
//    DeadEnds programs. Symbol tables are implented with hash tables.
//
// Created by Thomas Wetmore on 23 March 2023.
// Last changed on 1 June 2025.
//

#ifndef symboltable_h
#define symboltable_h

typedef struct PValue PValue;
typedef struct PNode PNode;
typedef struct Frame Frame;

#include "standard.h"
#include "hashtable.h"
#include "pnode.h"
#include "pvalue.h"
#include "frame.h"

// A SymbolTable holds DeadEnds script variables and their Pvalues.
typedef HashTable SymbolTable;

// A Symbol is an element in a SymbolTable; ident is the name of a variable/identifier, and value is its PValue.
typedef struct Symbol {
	CString ident;
	PValue *value;
} Symbol;

//  Interface to SymbolTable.
SymbolTable *createSymbolTable(void);
void deleteSymbolTable(SymbolTable*);
void assignValueToSymbol(Context*, CString, PValue);
void assignValueToSymbolTable(SymbolTable*, CString, PValue);
PValue getValueOfSymbol(Context*, CString);
PValue getValueFromSymbolTable(SymbolTable*, CString);
void showSymbolTable(SymbolTable*); // Debug.

#endif // symboltable_h
