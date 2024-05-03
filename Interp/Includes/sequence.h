// DeadEnds
//
// sequence.h is the header file for the Sequence datatype.
//
// Created by Thomas Wetmore on 1 March 2023.
// Last changed on 3 May 2024.

#ifndef sequence_h
#define sequence_h

#include "standard.h"
#include "block.h"
#include "gnode.h"
#include "pvalue.h"

// SortType holds the possible sorted states of the elements in a Sequence.
typedef enum {
	SequenceNotSorted,
	SequenceKeySorted,
	SequenceNameSorted,
	SequenceValueSorted
} SortType;

// SequenceEl is the type of Sequence elements.
typedef struct SequenceEl {
	//String key;
	//String name;
	GNode* root;
	String name;
	void* value;
} SequenceEl;

// Sequence is a data type that holds sequences/sets/arrays of persons.
typedef struct Sequence {
	Block block;
	SortType sortType;
	bool unique;
	Database *database;
} Sequence;

SequenceEl *createSequenceEl(Database*, String key, void* value);

Sequence *createSequence(Database*);
void deleteSequence(Sequence*);
Sequence *copySequence(Sequence*);
int lengthSequence(Sequence*);
void emptySequence(Sequence*);

void appendToSequence(Sequence*, CString key, void*);
bool isInSequence(Sequence*, String key);
bool removeFromSequence(Sequence*, String key);
bool removeFromSequenceByIndex (Sequence *sequence, int index);
void nameSortSequence(Sequence*); //  Sort by name.
void keySortSequence(Sequence*); //  Sort by key.
void valueSortSequence(Sequence*); //  Sort a sequence by value (not properly implemented).
Sequence *uniqueSequence(Sequence*);  //Return sequence uniqueued from another.
void uniqueSequenceInPlace(Sequence *sequence);  // Remove duplicate elements from a sequence.

Sequence *personToChildren(GNode *person, Database*);
Sequence *personToFathers(GNode *person, Database*);
Sequence *personToMothers(GNode *person, Database*);
Sequence *familyToChildren(GNode *family, Database*);
Sequence *familyToFathers(GNode *family, Database*);
Sequence *familyToMothers(GNode *family, Database*);
Sequence *personToSpouses(GNode *person, Database*);
Sequence *personToFamilies(GNode *person, bool, Database*);
Sequence *nameToSequence(CString, Database*);
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
bool elementFromSequence(Sequence* seq, int index, String* key, String* name);
void renameElementInSequence(Sequence* seq, String key);

void sequenceToGedcom(Sequence*, FILE*);
void showSequence(Sequence*);

bool limitPersonNode(GNode *node, int level);

//  FORSEQUENCE -- Macro that iterates over a sequence in its current order.
//--------------------------------------------------------------------------------------------------
// NOTE: depending on code inserted between FORSEQUENCE and ENDSEQUENCE, some
// variables might be unused, hence the ATTRIBUTE_UNUSED.

//  FORSEQUENCE and ENDSEQUENCE iterate a Sequence.
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
