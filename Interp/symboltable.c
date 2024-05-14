//
// DeadEnds
//
// symboltable.c holds the functions that implement SymbolTable.
//
// Created by Thomas Wetmore on 23 March 2023.
// Last changed on 3 April 2024.
//
#include <stdint.h>

#include "standard.h"
#include "refnindex.h"
#include "symboltable.h"

static bool debugging = false;

// globalTable holds the script symbols that defined in the global scope.
extern SymbolTable *globalTable;

// compare compares two symbols by ident fields.
static int compare(CString a, CString b) {
	return strcmp(a, b);
}

// delete deletes a Symbol.
static void delete(void* a) {
	Symbol *symbol = (Symbol*) a;
	free(symbol->value);
}

// getKey returns the Symbol's identifier.
static CString getKey(void *symbol) {
	return ((Symbol*) symbol)->ident;
}

// createSymbol creates a Symbol.
static Symbol *createSymbol(CString iden, PValue *ppvalue) {
	Symbol *symbol = (Symbol*) malloc(sizeof(Symbol));
	symbol->ident = iden;
	symbol->value = ppvalue;
	return symbol;
}

// createSymbolTable creates a SymbolTable.
static int numBucketsInSymbolTable = 37;
SymbolTable *createSymbolTable(void) {
	return createHashTable(getKey, compare, delete, numBucketsInSymbolTable);
}

// assignValueToSymbol assigns a value to a Symbol. If the Symbol isn't in the local table, look
// in the global table; only if there use the global table.
void assignValueToSymbol(SymbolTable *symtab, CString ident, PValue pvalue) {
	SymbolTable *table = symtab; // Determine SymbolTable.
	if (!isInHashTable(symtab, ident) && isInHashTable(globalTable, ident)) table = globalTable;
	PValue* ppvalue = (PValue*) malloc(sizeof(PValue)); // Copy pvalue to heap.
	memcpy(ppvalue, &pvalue, sizeof(PValue));
	if (pvalue.type == PVString)
		ppvalue->value.uString = strsave(pvalue.value.uString); // TODO: Is this required?
	Symbol *symbol = searchHashTable(table, ident);
	if (symbol) { // If symbol is in the table change its value.
		freePValue(symbol->value);
		symbol->value = ppvalue;
		return;
	}
	if (debugging) printf("assignValueToSymbol: %s = %s in %p\n", ident, pvalueToString(*ppvalue, true), symtab);
	addToHashTable(table, createSymbol(ident, ppvalue), true); // Otherwise add new symbol to table.
}

// getValueOfSymbol gets the value of a Symbol from a SymbolTable; Symbol's PValue is returned.
PValue getValueOfSymbol(SymbolTable *symtab, CString ident) {
	if (debugging) printf("getValueOfSymbol %s from table\n", ident);
	if (debugging) showSymbolTable(symtab);
	Symbol *symbol = searchHashTable(symtab, ident); // Search local table.
	if (symbol) {
		PValue *ppvalue = symbol->value;
		if (ppvalue) return (PValue){ppvalue->type, ppvalue->value};
	}
	symbol = searchHashTable(globalTable, ident); // Search global table.
	if (symbol) {
		PValue *ppvalue = symbol->value;
		if (ppvalue) return (PValue){ppvalue->type, ppvalue->value}; // Put on stack.
	}
	return nullPValue; // Undefined.
}

// showSymbolTable shows the contents of a SymbolTable.
void showSymbolTable(SymbolTable* table) {
	printf("Symbol Table at Location %p\n", table);
	for (int i = 0; i < table->numBuckets; i++) {
		Bucket *bucket = table->buckets[i];
		if (!bucket) continue;
		Block *block = &(bucket->block);
		if (block->length <= 0) continue;
		for (int j = 0; j < block->length; j++) {
			Symbol *symbol = (Symbol*) block->elements[j];
			String pvalue = pvalueToString(*(symbol->value), false);
			printf("  %s = %s\n", symbol->ident, pvalue);
		}
	}
}
