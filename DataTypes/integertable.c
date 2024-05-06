// DeadEnds
//
// integertable.c
//
// Created by Thomas Wetmore on 23 April 2023.
// Last changed on 1 May 2024.

#include "integertable.h"

CString integerGetKey(void* element) { return ((IntegerElement*) element)->key; }

// createIntegerTable creates and returns an IntegerTable.
IntegerTable *createIntegerTable(int numBuckets) {
    return createHashTable(integerGetKey, null, null, numBuckets);
}

// searchIntegerTable searches for a key in an IntegerTable and return its integer value.
int searchIntegerTable(IntegerTable *table, CString key) {
    IntegerElement* element = (IntegerElement*) searchHashTable(table, key);
    return element ? element->value : __INT_MAX__;
}

// insertInIntegerTable -- Insert a string key and integer value to an integer table.
void insertInIntegerTable(IntegerTable *table, CString key, int value) {
    IntegerElement* element = (IntegerElement*) searchHashTable(table, key);
    if (element) { // If there change value.
        element->value = value;
        return;
    }
    element = (IntegerElement*) malloc(sizeof(IntegerElement)); // Create new element.
    element->key = key;
    element->value = value;
    addToHashTable(table, element, false);
}

// incrIntegerTable -- retrieve key and increment its value.
void incrIntegerTable (IntegerTable *table, CString key)
{
  IntegerElement *element = (IntegerElement *) searchHashTable (table, key);
  int value;

  if (element)
    value = element->value + 1;
  else
    value = 1;
 
  insertInIntegerTable (table, key, value);
}
