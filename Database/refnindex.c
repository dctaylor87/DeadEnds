//
//  DeadEnds Project
//
//  refnindex.c -- Handle user reference indexing.
//
//  Gedcom records can have 1 REFN nodes whose values give records unique identifiers. A record
//  can have more than one 1 REFN node with different values.
//
//  Created by Thomas Wetmore on 16 December 2023.
//  Last changed on 25 January 2024.
//

#include <stdint.h>

#include "refnindex.h"
#include "gedcom.h"

//  searchRefnIndex -- Search a RefnIndex for a reference (REFN) value.
//--------------------------------------------------------------------------------------------------
CString searchRefnIndex(RefnIndex *index, CString refn)
//  index -- Reference index to search
//  refn -- Reference value to search for
//  Returns the key of the record with the reference, if exists, or null otherwise
{
	RefnIndexEl *el = (RefnIndexEl*) searchHashTable(index, refn);
	return el ? el->key : null;
}

//  createRefnIndexEl -- Create a new reference index entry.
//-------------------------------------------------------------------------------------------------
RefnIndexEl *createRefnIndexEl(CString refn, CString key)
{
	RefnIndexEl *el = (RefnIndexEl*) stdalloc(sizeof(RefnIndexEl));
	el->refn = refn;
	el->key = key;
	return el;
}

//  showRefnIndex -- Show a RefnIndex, for debugging.
//--------------------------------------------------------------------------------------------------
void showRefnIndex(RefnIndex *index)
{
	printf("showRefnIndex: Write me\n");
}

//  cmpRefnIndexEls -- Compare function for RefnIndexEls.
//--------------------------------------------------------------------------------------------------
static int cmpRefnIndexEls (Word a, Word b)
{
	return strcmp(((RefnIndexEl*) a)->key, ((RefnIndexEl*) b)->key);
}

//  getRefnIndexElKey -- Get key function for RefnIndexEls.
//--------------------------------------------------------------------------------------------------
static String getRefnIndexElKey (Word a)
{
	return ((RefnIndexEl*) a)->key;
}

//  createRefnIndex -- Create a RefnIndex.
//--------------------------------------------------------------------------------------------------
RefnIndex *createRefnIndex(void)
{
	return (RefnIndex*) createHashTable(cmpRefnIndexEls, null, getRefnIndexElKey);
}

//  deleteRefnIndex -- Delete a RefnIndex.
//--------------------------------------------------------------------------------------------------
void deleteRefnIndex (RefnIndex *index)
{
	deleteHashTable(index);
}

//  insertInRefnIndex -- Insert a new reference entry in the RefnIndex. Returns true if the the
//    reference value was not already in the index. Returns false of the reference value is
//    already in the index.
//-------------------------------------------------------------------------------------------------
bool insertInRefnIndex (RefnIndex *index, CString refn, CString key)
{
	//  Get the bucket index and create a bucket if it does not exist.
	int hash = getHash(refn);
	Bucket *bucket = index->buckets[hash];
	if (!bucket) {
		bucket = createBucket();
		index->buckets[hash] = bucket;
	}
	//  See if the reference (REFN) value is already in the index.
	if (searchRefnIndex(index, refn)) return false;
	insertInHashTable(index, createRefnIndexEl(refn, key));
	return true;
}
