// DeadEnds
//
// sequence.h is the header file for the Sequence datatype.
//
// Created by Thomas Wetmore on 1 March 2023.
// Last changed on 20 April 2024.

#ifndef sequence_h
#define sequence_h

#include "standard.h"
#include "block.h"
#include "gnode.h"
//#include "pnode.h"
#include "pvalue.h"

// SortType holds the possible sorted states of the elements in a Sequence.
typedef enum {
	SequenceNotSorted,
	SequenceKeySorted,
	SequenceNameSorted,
	SequenceValueSorted
} SortType;

// SequenceEl is the data type of Sequence elements. Keys and names belong to the element.
typedef struct SequenceEl {
	String key;  // Person or family key.
	String name; // Name of person.
	void* value;   // Caller defined.
} SequenceEl;

// Sequence is a data type that holds sequences/sets/arrays of persons (families?).
typedef struct Sequence {
	Block block;         // Block holding the SequenceEls.
	SortType sortType;   // State of sortedness.
	bool unique;         // Sequence has been uniqued.
	Database *database;  // Database the sequence comes from.
} Sequence;

SequenceEl *createSequenceEl(String key, String name, void* value);

Sequence *createSequence(Database*);  // Create a sequence.
void deleteSequence(Sequence*);  //  Delete a sequence.
Sequence *copySequence(Sequence*);  //  Copy a sequence.
int lengthSequence(Sequence*);  //  Return the length of a sequence.
void emptySequence(Sequence*);

void appendToSequence(Sequence*, CString key, String name, void*);
bool isInSequence(Sequence*, String key);
bool removeFromSequence(Sequence*, String key);
bool removeFromSequenceByIndex (Sequence *sequence, int index);
void nameSortSequence(Sequence*);  //  Sort a sequence by name.
void keySortSequence(Sequence*);  //  Sort a sequence by key.
void valueSortSequence(Sequence*); //  Sort a sequence by value (not properly implemented).
Sequence *uniqueSequence(Sequence*);  //  Return sequence uniqueued from another.
void uniqueSequenceInPlace(Sequence *sequence);  // Remove duplicate elements from a sequence.

Sequence *personToChildren(GNode *person, Database*);  //  Return sequence of a person's children.
Sequence *personToFathers(GNode *person, Database*);   //  Return sequence of a person's fathers.
Sequence *personToMothers(GNode *person, Database*);   //  Return sequence of a person's mothers.
Sequence *familyToChildren(GNode *family, Database*);  //  Return sequence of a family's children.
Sequence *familyToFathers(GNode *family, Database*);  //  Return sequence of a family's fathers.
Sequence *familyToMothers(GNode *family, Database*);  //  Return sequence of a family's mothers.
Sequence *personToSpouses(GNode *person, Database*);  //  Return sequence of a person's spouses.
Sequence *personToFamilies(GNode *person, bool, Database*);  //  Return sequence of a person's families.
Sequence *nameToSequence(CString, Database*);  //  Return sequence of persons who match a name.
#if defined(DEADENDS)
Sequence *refnToSequence (CString ukey, Database *database);
#else
Sequence *refn_to_indiseq(String refn);
#endif

Sequence *unionSequence(Sequence*, Sequence*);
Sequence *intersectSequence(Sequence*, Sequence*);
Sequence *differenceSequence(Sequence*, Sequence*);
Sequence *childSequence(Sequence*);
Sequence *parentSequence(Sequence*);
Sequence *spouseSequence(Sequence*);
Sequence *ancestorSequence(Sequence*);
Sequence *descendentSequence(Sequence*);
Sequence *siblingSequence(Sequence*, bool);

void renameSequence(Sequence *seq, String key);
bool elementSequence(Sequence *seq, int index, String* pkey, String* pname);
CString elementKeySequence (Sequence *seq, int index);

void sequenceToGedcom(Sequence*, FILE*);
bool limitPersonNode(GNode *node, int level);

// Kind of for debugging.
void showSequence(Sequence*);

//  FORSEQUENCE -- Macro that iterates over a sequence in its current order.
//--------------------------------------------------------------------------------------------------
// NOTE: depending on code inserted between FORSEQUENCE and ENDSEQUENCE, some
// variables might be unused, hence the ATTRIBUTE_UNUSED.

//  FORSEQUENCE and ENDSEQUENCE are macros that iterate over a Sequence.
#define FORSEQUENCE(sequence, element, count)\
{\
	SequenceEl *element;\
	int count;\
	Block *___block = &(sequence->block);\
	SequenceEl **__elements = (SequenceEl**) ___block->elements;\
	for (int __i = 0; __i < ___block->length; __i++){\
		element = __elements[__i];\
		count = __i + 1;

#define ENDSEQUENCE }}

#endif // sequence_h
