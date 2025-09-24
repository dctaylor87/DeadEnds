//
//  DeadEnds Library
//
//  sequence.c holds the functions that implement the Sequence data type that handles sets of
//  persons and other record types. It underlies the indiseq data type of DeadEnds Script.
//
//  Created by Thomas Wetmore on 1 March 2023.
//  Last changed on 25 July 2025.
//

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "standard.h"
#include "gedcom.h"
#include "gnode.h"
#include "hashtable.h"
#include "list.h"
#include "name.h"
#include "sequence.h"
#include "sort.h"
#include "stringtable.h"

//static bool debugging = false;
// keyGetKey is the getKey function that returns the key of a SequenceEl.
static CString keyGetKey(const void* element) {
	SequenceEl* el = (SequenceEl*) element;
	return el->root->key;
}

// nameGetKey is the getKey function that returns the name of a SequenceEl.
static CString nameGetKey(const void* element) {
	SequenceEl* el = (SequenceEl*) element;
	return el->name;
}

// keyCompare is the compare function that compares SequenceEl's by key.
static int keyCompare(CString a, CString b) {
	return compareRecordKeys(a, b);
}

// nameCompare is the compare function that compares SequenceEl's by name.
static int nameCompare(CString a, CString b) {
	return compareNames(a, b);
}

// delete is the function that deletes a SequenceEl.
static void delete(void* element) {
	stdfree(element);
}

void baseFree(void *word) { stdfree(word); }

// sequenceElements returns the array of elments in a Sequence.
static SequenceEl** sequenceElements(Sequence *sequence) {
	return (SequenceEl**) (&(sequence->block))->elements;
}

// createSequenceEl creates a SequenceEl.
//SequenceEl* createSequenceEl(Database* database, CString key, void* value) {
SequenceEl* createSequenceEl(RecordIndex* index, CString key, const void* value) {
	SequenceEl* element = (SequenceEl*) stdalloc(sizeof(SequenceEl));
	GNode* root = getRecord(key, index);
	ASSERT(root);
	if (! element)
	  return NULL;
	memset(element, 0, sizeof(SequenceEl));
	element->root = root;
	if (recordType(root) == GRPerson) element->name = (NAME(root))->value;
	element->value = value;
	return element;
}

// createSequence creates a Sequence.
//Sequence* createSequence(Database* database) {
Sequence* createSequence(RecordIndex* index) {
	Sequence* sequence = (Sequence*) stdalloc(sizeof(Sequence));
	memset(sequence, 0, sizeof(Sequence));
	initBlock(&(sequence->block));
	//sequence->database  = database;
	sequence->index  = index;
	sequence->unique = false;
	sequence->sortType = SequenceNotSorted;
	return sequence;
}

// lengthSequence returns the length of a Sequence.
int lengthSequence(Sequence* sequence) {
	return (&(sequence->block))->length;
}

// deleteSequence deletes a Sequence.
void deleteSequence(Sequence* sequence) {
	Block* block = &(sequence->block);
	deleteBlock(block, delete);
	stdfree(sequence);
}

// emptySequence removes the elements from a Sequence.
void emptySequence(Sequence *sequence) {
	Block* block = &(sequence->block);
	emptyBlock(block, delete);
}

// appendToSequence creates and appends a SequenceEl to a Sequence.
void appendToSequence(Sequence* sequence, CString key, const void* value) {
	if (!sequence || !key) return;
	//SequenceEl* element = createSequenceEl(sequence->database, key, value);
	SequenceEl* element = createSequenceEl(sequence->index, key, value);
	appendToBlock(&(sequence->block), element);
    sequence->sortType = SequenceNotSorted;
}

// appendSequenceToSequence appends a Sequence to another Sequence. The Sequences must be distinct.
// destination changes; source does not.
void appendSequenceToSequence(Sequence* destination, Sequence* source) {
	FORSEQUENCE(source, element, count)
		appendToSequence(destination, element->root->key, element->value);
	ENDSEQUENCE
    destination->sortType = SequenceNotSorted;
}

// renameElementInSequence updates an element in a Sequence with a new name.
void renameElementInSequence(Sequence* sequence, CString key) {
	if (!sequence || !key) return;
	Block *block = &(sequence->block);
	SequenceEl** elements = (SequenceEl**) block->elements;
	for (int i = 0; i < block->length; i++) {
		GNode* root = elements[i]->root;
		if (eqstr(key, root->key)) {
			(elements[i])->name = NAME(root)->value;
		}
	}
    if (sequence->sortType == SequenceNameSorted)
        sequence->sortType = SequenceNotSorted;
}

// isInSequence checks if a SequenceEl with given key is in a Sequence.
// TODO: Should take advantage of the sortability of the Sequence.
bool isInSequence(Sequence *seq, CString key) {
	if (!seq || !key) return false;
	Block *block = &(seq->block);
	SequenceEl **elements = (SequenceEl**) block->elements;
	for (int i = 0; i < block->length; i++) {
		if (eqstr(key, (elements[i])->root->key)) return true;
	}
	return false;
}

// removeFromSequence removes the SequenceEl with the given key from the Sequence.
bool removeFromSequence(Sequence *sequence, CString key) {
	ASSERT(sequence && key);
	if (!sequence || !key) return false;
	Block *block = &(sequence->block);
	if (sequence->sortType == SequenceNotSorted) {
		return removeFromUnsortedBlock(block, key, keyGetKey, delete);
	} else if (sequence->sortType == SequenceKeySorted) {
		return removeFromSortedBlock(block, key, keyGetKey, keyCompare, delete);
	} else if (sequence->sortType == SequenceNameSorted) {
		return removeFromSortedBlock(block, key, nameGetKey, nameCompare, delete);
	} else {
		printf("NO OTHER SORT TYPE SUPPORTED YET\n");
	}
	return false;
}

// removeFromSequenceByIndex removes the SequenceEl having the given index
bool removeFromSequenceByIndex (Sequence *sequence, int index)
{
  CString key;

  if (! sequence)
    return false;		/* no sequence */
  if (! elementFromSequence (sequence, index, &key, NULL))
    return false;		/* index out of range */

  return removeFromSequence (sequence, key);
}

// elementFromSequence returns the key and name values of an indexed Sequence element.
bool elementFromSequence (Sequence* sequence, int index, CString* pkey, CString* pname) {
	ASSERT(sequence);
	Block *block = &(sequence->block);
	if (index < 0 || index >= block->length) return false;
	if (pkey) *pkey = keyGetKey((block->elements)[index]);
	if (pname) *pname = nameGetKey((block->elements)[index]);
	return true;
}

// nameSortSequence sorts a sequence by the names of the persons. Assumes person Sequence.
void nameSortSequence(Sequence* sequence) {
	if (sequence->sortType == SequenceNameSorted) return;
	sortBlock(&(sequence->block), nameGetKey, nameCompare);
	sequence->sortType = SequenceNameSorted;
}

// keySortSequence sorts a Sequence by key.
void keySortSequence(Sequence* sequence) {
	if (sequence->sortType == SequenceKeySorted) return;
	sortBlock(&(sequence->block), keyGetKey, keyCompare);
	sequence->sortType = SequenceKeySorted;
}

// copySequence creates a copy of the given Sequence.
Sequence* copySequence(Sequence* sequence) {
	Sequence* copy = createSequence(sequence->index);
	FORSEQUENCE(sequence, element, count)
		appendToSequence(copy, element->root->key, element->value);
	ENDSEQUENCE
	return copy;
}

// uniqueSequence returns a Sequence with the unique elements from the given Sequence.
// MNOTE: creates a new Sequence, so caller may need to free the original.
Sequence* uniqueSequence(Sequence* sequence) {
	ASSERT(sequence);
	Block* block = &(sequence->block);
	Sequence* unique = createSequence(sequence->index);
	if (block->length == 0) return unique;
	int n = block->length;
	Block *uBlock = &(unique->block);
	if (sequence->sortType != SequenceKeySorted) keySortSequence(sequence);
	SequenceEl** els = (SequenceEl**) block->elements;
	SequenceEl* el = els[0];
	RecordIndex* index = sequence->index;
	appendToBlock(uBlock, createSequenceEl(index, el->root->key, el->value));
	int i, j;
	for (j = 0, i = 1; i < n; i++) {
		if (nestr(els[i]->root->key, els[j]->root->key)) {
			appendToBlock(uBlock, createSequenceEl(index, els[i]->root->key, els[i]->value));
			j = i;
		}
	}
	return unique;
}

// uniqueSequenceInPlace removes duplicate (have the same key) elements from a Sequence.
void uniqueSequenceInPlace(Sequence* sequence) {
	if (!sequence) return;
	Block* block = &(sequence->block);
	int n = block->length;
	if (n <= 1) return;
	if (sequence->sortType != SequenceKeySorted) keySortSequence(sequence);
	SequenceEl** els = (SequenceEl**) block->elements;
	int i, j;
	for (j = 0, i = 1; i < n; i++)
		if (nestr(els[i]->root->key, els[j]->root->key)) els[++j] = els[i];
	block->length = j + 1;
}

// unionSequence returns the union of two Sequences.
Sequence* unionSequence(Sequence* one, Sequence* two) {
	if (!one || !two || one->index != two->index) return null;
	if (one->sortType != SequenceKeySorted) keySortSequence(one);
	if (two->sortType != SequenceKeySorted) keySortSequence(two);
	if (!one->unique) uniqueSequenceInPlace(one);
	if (!two->unique) uniqueSequenceInPlace(two);
	int n = lengthSequence(one);
	int m = lengthSequence(two);
	Sequence* three = createSequence(one->index);
	SequenceEl** u = sequenceElements(one);
	SequenceEl** v = sequenceElements(two);
	int i = 0, j = 0, rel;
	while (i < n && j < m) {
		if ((rel = keyCompare(u[i]->root->key, v[j]->root->key)) < 0) {
			appendToSequence(three, u[i]->root->key, u[i]->value);
			i++;
		} else if (rel > 0) {
			appendToSequence(three, v[j]->root->key, v[j]->value);
			j++;
		} else {
			appendToSequence(three, u[i]->root->key, u[i]->value);
			i++; j++;
		}
	}
	while (i < n) {
		appendToSequence(three, u[i]->root->key, u[i]->value);
		i++;
	}
	while (j < m) {
		appendToSequence(three, v[j]->root->key, v[j]->value);
		j++;
	}
	three->sortType = SequenceKeySorted;
	three->unique = true;
	return three;
}

// intersectSequence returns the intersection of two Sequences.
Sequence* intersectSequence(Sequence* one, Sequence* two) {
	ASSERT(one && two);
	ASSERT(one->index == two->index);
	if (!one || !two || one->index != two->index) return null;
	int rel;
	if (one->sortType != SequenceKeySorted) keySortSequence(one);
	if (two->sortType != SequenceKeySorted) keySortSequence(two);
	if (!one->unique) uniqueSequenceInPlace(one);
	if (!two->unique) uniqueSequenceInPlace(two);
	int n = lengthSequence(one);
	int m = lengthSequence(two);
	Sequence* three = createSequence(one->index);
	int i = 0, j = 0;
	SequenceEl** u = sequenceElements(one);
	SequenceEl** v = sequenceElements(two);
	while (i < n && j < m) {
		if ((rel = compareRecordKeys(u[i]->root->key, v[j]->root->key)) < 0) {
			i++;
		} else if (rel > 0) {
			j++;
		} else {
			appendToSequence(three, (u[i])->root->key, u[i]->value);
			i++; j++;
		}
	}
	three->sortType = SequenceKeySorted;
	three->unique = true;
	return three;
}

// differenceSequence returns the difference of two Sequences.
Sequence* differenceSequence(Sequence* one, Sequence* two) {
	ASSERT(one && two);
	ASSERT(one->index == two->index);
	if (!one || !two) return null;
	if (one->sortType != SequenceKeySorted) keySortSequence(one);
	if (two->sortType != SequenceKeySorted) keySortSequence(two);
	if (!one->unique) uniqueSequenceInPlace(one);
	if (!two->unique) uniqueSequenceInPlace(two);
	int n = lengthSequence(one);
	int m = lengthSequence(two);
	Sequence* three = createSequence(one->index);
	int i = 0, j = 0;
	SequenceEl** u = sequenceElements(one);
	SequenceEl** v = sequenceElements(two);
	int rel;
	while (i < n && j < m) {
		if ((rel = compareRecordKeys(u[i]->root->key, v[j]->root->key)) < 0) {
			appendToSequence(three, u[i]->root->key, u[i]->value);
			i++;
		} else if (rel > 0) {
			j++;
		} else {
			i++; j++;
		}
	}
	while (i < n) {
		appendToSequence(three, u[i]->root->key, u[i]->value);
		i++;
	}
	three->sortType = SequenceKeySorted;
	three->unique = true;
	return three;
}

// showSequence is a debug function that shows the contents of a Sequence.
void showSequence(Sequence* sequence, String title) {
	if (title) printf("%s:\n", title);
	FORSEQUENCE(sequence, element, count)
		printf("%d: %s: %s\n", count, element->root->key, element->name ? element->name : "no name");
	ENDSEQUENCE

}

void incrReferenceCountSequence (Sequence *seq,
				 CString file, int line, CString function)
{
  seq->refCount++;
  logRefCountChange ((void *)seq, "Sequence", seq->refCount,
		     file, line, function);
}
