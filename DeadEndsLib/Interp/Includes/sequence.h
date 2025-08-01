//
//  DeadEnds Library
//
//  sequence.h is the header file for the Sequence datatype.
//  NOTE: This datatype is located in the Interp subdirectory of the DeadEndsLib because it is
//  the data structure underlyihg the INDISEQ data type of DEScript. However, it could easily be
//  used as a more general purpose data structure.
//
//  Created by Thomas Wetmore on 1 March 2023.
//  Last changed on 17 June 2025.
//

#ifndef sequence_h
#define sequence_h

#include "block.h"

typedef struct GNode GNode;
typedef struct HashTable HashTable;
typedef HashTable RecordIndex;
typedef HashTable NameIndex;
typedef HashTable RefnIndex;
typedef struct Block Block;
typedef struct PValue PValue;

// SortType holds the possible sorted states of a Sequence.
typedef enum {
	SequenceNotSorted,
	SequenceKeySorted,
	SequenceNameSorted,
} SortType;

// SequenceEl is the type of Sequence elements.
typedef struct SequenceEl {
	GNode* root; // Root of record. MNOTE: do not free on delete.
	String name; // If element is a person. MNOTE: do not free on delete.
	PValue* value; // MNOTE: what do we do with this? Scenarios exists for do or not do free.
} SequenceEl;

// Sequence is a data type that holds sequences/sets/arrays of records.
typedef struct Sequence {
	Block block;
	int refCount; // Number of references to the sequence
	SortType sortType;
	bool unique;
	RecordIndex* index;
} Sequence;

SequenceEl* createSequenceEl(RecordIndex*, CString key, const void* value);
Sequence* createSequence(RecordIndex*);
void deleteSequence(Sequence*);
Sequence* copySequence(Sequence*);
int lengthSequence(Sequence*);
void emptySequence(Sequence*);

void appendToSequence(Sequence*, CString key, const void*);
void appendSequenceToSequence(Sequence* dst, Sequence* src);
bool isInSequence(Sequence*, CString key);
bool removeFromSequence(Sequence*, CString key);
bool removeFromSequenceByIndex (Sequence *sequence, int index);
void nameSortSequence(Sequence*);
void keySortSequence(Sequence*);
//void valueSortSequence(Sequence*);
Sequence* uniqueSequence(Sequence*);
void uniqueSequenceInPlace(Sequence *sequence);

Sequence* personToChildren(GNode* person, RecordIndex*);
Sequence* personToFathers(GNode* person, RecordIndex*);
Sequence* personToMothers(GNode* person, RecordIndex*);
Sequence* familyToChildren(GNode* family, RecordIndex*);
Sequence* familyToFathers(GNode* family, RecordIndex*);
Sequence* familyToMothers(GNode* family, RecordIndex*);
Sequence* personToSpouses(GNode* person, RecordIndex*);
Sequence* personToFamilies(GNode* person, bool, RecordIndex*);
Sequence* nameToSequence(CString name, RecordIndex*, NameIndex*);
Sequence* keyToSequence(CString key, RecordIndex*);
Sequence* refnToSequence (CString value, RecordIndex*, RefnIndex*);
Sequence* stringToSequence(CString name, Database* database);

Sequence* unionSequence(Sequence*, Sequence*);
Sequence* intersectSequence(Sequence*, Sequence*);
Sequence* differenceSequence(Sequence*, Sequence*);
Sequence* childSequence(Sequence*);
Sequence* parentSequence(Sequence*);
Sequence* spouseSequence(Sequence*);
Sequence* ancestorSequence(Sequence*, bool close);
Sequence* descendentSequence(Sequence*, bool close);
Sequence* siblingSequence(Sequence*, bool close);
bool elementFromSequence(Sequence* sequence, int index, CString* key, CString* name);
void renameElementInSequence(Sequence* sequence, CString key);
void sequenceToGedcom(Sequence*, FILE*);
void showSequence(Sequence*, String title);

bool limitPersonNode(GNode *node, int level);

extern void baseFree(void *word);
extern void incrReferenceCountSequence (Sequence *seq, CString file, int line, CString function);

// FORSEQUENCE and ENDSEQUENCE iterate a Sequence.
#define FORSEQUENCE(sequence, element, count) {\
	SequenceEl* element;\
	int count ATTRIBUTE_UNUSED;\
	Block* ___block = &(sequence->block);\
	SequenceEl** __elements = (SequenceEl**) ___block->elements;\
	for (int __i = 0; __i < ___block->length; __i++){\
		element = __elements[__i];\
		count = __i + 1;

#define ENDSEQUENCE }}

#define INCRSEQREFCOUNT(seq)	incrReferenceCountSequence (seq, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__)

#endif // sequence_h
