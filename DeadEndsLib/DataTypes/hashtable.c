// DeadEnds
//
// hashtable.c implements a general hash table. Specialized hash tables are created through
// customiziing the compare, delete and getKey functions.
//
// Created by Thomas Wetmore on 29 November 2022.
// Last changed on 20 May 2025.

#include "standard.h"
#include "hashtable.h"
#include "sort.h"

bool debuggingHash = false;
bool sortChecking = false;

static void* searchBucket(Bucket*, CString key, CString(*g)(void*), int(*c)(CString, CString), int* index);

// createHashTable creates and returns a HashTable. getKey is a function that returns the key of
// an element, and delete is an optional function that frees an element.
HashTable* createHashTable(CString(*getKey)(void*), int(*compare)(CString, CString),
						   void(*delete)(void*), int numBuckets) {
	HashTable *table = (HashTable*) stdalloc(sizeof(HashTable));
	if (! table)
	  return NULL;
	memset(table, 0, sizeof(HashTable));
	table->compare = compare;
	table->delete = delete;
	table->getKey = getKey;
	table->numBuckets = numBuckets;
	table->buckets = (Bucket**) stdalloc(numBuckets*sizeof(Bucket));
	if (! table->buckets)
	{
	  stdfree (table);
	  return NULL;
	}
	for (int i = 0; i < table->numBuckets; i++) table->buckets[i] = null;
	table->refcount = 1;
	return table;
}

// deleteBucket deletes a Bucket. If there is a delete function it is called on the elements.
static void deleteBucket(Bucket* bucket, void(*delete)(void*)) { //PH;
	if (delete) {
		Block* block = &(bucket->block);
		for (int j = 0; j < block->length; j++) {
			delete(block->elements[j]);
		}
	}
	stdfree(bucket);
}

// deleteHashTable deletes a HashTable. If there is a delete function it is called on the elements.
void deleteHashTable(HashTable *table) { //PH;
	if (!table) return;
	for (int i = 0; i < table->numBuckets; i++) {
		if (table->buckets[i] == null) continue;
		deleteBucket(table->buckets[i], table->delete);
	}
	stdfree(table);
}

// createBucket creates and returns an empty Bucket.
Bucket *createBucket(void) { //PH;
	Bucket *bucket = (Bucket*) stdalloc(sizeof(Bucket));
	if (! bucket)
	  return NULL;
	memset(bucket, 0, sizeof(Bucket));
	initBlock(&(bucket->block));
	return bucket;
}

// lengthBucket returns the length of a Bucket.
int lengthBucket(Bucket* bucket) {
	return lengthBlock(&(bucket->block));
}



// getHash returns the hash code of a Strings; found on the internet.
int getHash(CString key, int maxHash) { //PH;
	unsigned long hash = 5381;
	int c;
	CString p = key;
	while ((c = *p++)) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	hash = hash & 0x0000efff;
	hash = hash % maxHash;
	return (int) hash;
}

// detailSearchHashTable is a static function at the bottom of HashTable's search stack.
// NOTE: It may be better to return the Bucket rather than the hash.
void* detailSearchHashTable(HashTable* table, CString key, int* phash, int* pindex) { //PH;
	int hash = getHash(key, table->numBuckets);
	if (phash) *phash = hash;
	Bucket* bucket = table->buckets[hash];
	if (!bucket) { // Bucket doesn't exist.
		if (pindex) *pindex = 0;
		return null;
	}
	//printf("detailSearchHashTable calling searchBucket: hash %d; length %d, key: %s\n", hash, lengthBucket(bucket), key); // DEBUG
	return searchBucket(bucket, key, table->getKey, table->compare, pindex); // Bucket exists.
}

// searchHashTable searches a HashTable for the element with given key. It returns the element
// if found or null otherwise.
void* searchHashTable(HashTable* table, CString key) { //PH;
	return detailSearchHashTable(table, key, null, null);
}

// searchHashTableWithElement searches a HashTable for the element with the same key as the given
// element.
void* searchHashTableWithElement(HashTable* table, void* element) {
	return detailSearchHashTable(table, table->getKey(element), null, null);
}

// searchBucket searches a Bucket for an element by key. Depending on Bucket size either linear or
// binary search is used.
void* searchBucket(Bucket* bucket, CString key, CString(*getKey)(void*),
				   int(*compare)(CString, CString), int* index) { //PH;
	return searchBlock(&(bucket->block), key, getKey, index);
}

// linearSearchBucket uses linear search to look for an element in a Bucket.
//void* linearSearchBucket(Bucket* bucket, CString key, CString(*getKey)(void*), int* index) { PH;
       // Block* block = &(bucket->block);
       // return linearSearch(block->elements, block->length, key, getKey, index);
//}

// isInHashTable returns whether an element with the given key is in the HashTable.
bool isInHashTable(HashTable* table, CString key) {
	return detailSearchHashTable(table, key, null, null) != null;
}

// addToHashTable adds a new element to a HashTable.
void addToHashTable(HashTable* table, void* element, bool replace) { //PH;
	//printf("addToHashTable called element with key %s\n", table->getKey(element)); // DEBUG
	CString key = table->getKey(element);
	int hash, index;
	Bucket* bucket = null;
	bool found = detailSearchHashTable(table, key, &hash, &index);
	if (found && replace) { // Replace existing element.
		bucket = table->buckets[hash];
		setBlockElement(&(bucket->block), element, table->delete, index);
		return;
	}
	if (found) {
		//printf("Duplicate key: %s found when adding to hash table\n", table->getKey(element)); // DEBUG
		return; // Element exists, but don't replace.
	}
	bucket = table->buckets[hash]; // Add it; be sure Bucket exists.
	if (!bucket) {
		bucket = createBucket();
		table->buckets[hash] = bucket;
	}
	appendToBlock(&(bucket->block), element);
}

bool addToHashTableIfNew(HashTable* table, void* element) { //PH;
	// See if it is there and if not where it should go if the bucket is sorted.
	// If there return false, meaning didn't add it.
	// Put the element into the right place.
	return true;
}

// removeFromHashTable removes the element with given key from a HashTable.
void removeFromHashTable(HashTable* table, CString key) { //PH;
	int hash = getHash(key, table->numBuckets);
	Bucket *bucket = table->buckets[hash];
	if (!bucket) return /*false*/;
	Block *block = &(bucket->block);
	int index = 0;
	void *element = linearSearch(block->elements, block->length, key, table->getKey, &index);
	if (element) {
		ASSERT(index != -1);
		removeFromBlock(block, index, table->delete);
	}
}

// appendToBucket adds a new element to the end of a bucket.
void appendToBucket(Bucket* bucket, void* element) { //PH;
	appendToBlock(&(bucket->block), element);
}

// removeElement removes an element from a hash table. It does not use binary search in cases
// when it should.
// TODO: GET BINARY SEARCH WORKING IF LENGTH IS OVER THRESHHOLD.
void removeElement(HashTable* table, void *element) { //PH;
	CString key = table->getKey(element);
	Bucket *bucket = table->buckets[getHash(key, table->numBuckets)];
	Block *block = &(bucket->block);
	void **elements = block->elements;
	int length = block->length;
	int i = 0;
	for (; i < length; i++) {
		CString check = table->getKey(elements[i]);
		if (eqstr(key, check)) break;
	}
	if (i >= length) return; // without doing anything.
	if (table->delete) table->delete(elements[i]);
	for (; i < length - 1; i++)
		elements[i] = elements[i+1];
	block->length--;
}

//  sizeHashTable returns the size (number of elements) in a hash table.
int sizeHashTable(HashTable* table) { //PH;
	int length = 0;
	for (int i = 0; i < table->numBuckets; i++) {
		if (table->buckets[i]) {
			Block *block = &((table->buckets[i])->block);
			length += block->length;
		}
	}
	return length;
}

// firstInHashTable returns the first element in a hash table; it works with nextInHashTable to
// iterate the table, returning each element in turn. The (in, out) variables keep track of the
// iteration state. The caller must provide the locations to two integer varibalbes to hold the
// stage. This function and nextInHashTable must be called from the same function. Macros
// FORHASHTABLE and ENDHASHTABLE are available to simplify calling these two functions.
void* firstInHashTable(HashTable* table, int* bucketIndex, int* elementIndex) { //PH;
	for (int i = 0; i < table->numBuckets; i++) {
		Bucket* bucket = table->buckets[i];
		if (bucket == null) continue;
		*bucketIndex = i;
		*elementIndex = 0;
		Block *block = &(bucket->block);
		return block->elements[0];
	}
	return null;
}

// nextInHashTable returns the next element in the hash table, using the (in,out) state
// variables to keep track of the state of the iteration.
void* nextInHashTable(HashTable* table, int* bucketIndex, int* elementIndex) { //PH;
	Bucket* bucket = table->buckets[*bucketIndex];
	Block* block = &(bucket->block);
	if (*elementIndex < block->length - 1) {
		*elementIndex += 1;
		return block->elements[*elementIndex];
	}
	// Reached end of current Bucket; find next.
	for (int i = *bucketIndex + 1; i < table->numBuckets; i++) {
		bucket = table->buckets[i];
		if (bucket == null) continue;  // 'Empty' bucket.
		*bucketIndex = i;
		*elementIndex = 0;
		block = &(bucket->block);
		return block->elements[0];
	}
	return null; // No more elements.
}

// lastInHashTable returns the last element in a hash table; it works
// with previousInHashTable to iterate the table, returning each
// element in turn. The (in, out) variables keep track of the
// iteration state. The caller must provide the locations of two
// integer varibalbes to hold the state.

void* lastInHashTable(HashTable* table, int* bucketIndex, int* elementIndex) { //PH;
	for (int i = table->numBuckets - 1; i >= 0; i--) {
		Bucket* bucket = table->buckets[i];

		if (bucket == null) continue;
		*bucketIndex = i;

		Block *block = &(bucket->block);
		*elementIndex = block->length - 1;

		return block->elements[block->length - 1];
	}
	return null;
}

// previousInHashTable returns the next element in the hash table, using the (in,out) state
// variables to keep track of the state of the iteration.
void* previousInHashTable(HashTable* table, int* bucketIndex, int* elementIndex) { //PH;
	Bucket* bucket = table->buckets[*bucketIndex];
	Block* block = &(bucket->block);
	if (*elementIndex > 0) {
		*elementIndex -= 1;
		return block->elements[*elementIndex];
	}
	// Reached start of current Bucket; find previous;
	for (int i = *bucketIndex - 1; i >= 0; i--) {
		bucket = table->buckets[i];
		if (bucket == null) continue;  // 'Empty' bucket.
		*bucketIndex = i;
		*elementIndex = block->length - 1;
		block = &(bucket->block);
		return block->elements[block->length - 1];
	}
	return null; // No more elements.
}

// iterateHashTable iterates a hash table and performs a function on each element; elements
// are visited in hash key order.
void iterateHashTable(HashTable* table, void (*function)(void*)) {//PH;
	if (!function) return;
	FORHASHTABLE(table, element)
		(*function)(element);
	ENDHASHTABLE
}

// iterateHashTableWithPredicate iterates a hash table running a predicate on each; elements
// are visited in hash key order; returns the number of elements that match the predicate.
int iterateHashTableWithPredicate(HashTable* table, bool (*predicate)(void*)) { //PH;
	int count = 0;
	FORHASHTABLE(table, element)
		if ((*predicate)(element)) count++;
	ENDHASHTABLE
	return count;
}

// iterateHashTableWithPredicate2 is like
// iterateHashTableWithPredicate except that the predicate takes an
// extra argument -- to avoid the need for extra variables.
int iterateHashTableWithPredicate2 (HashTable* table, void *predicateArg,
				    bool (*predicate)(void *elt, void *extra))
{
  int count = 0;
  FORHASHTABLE(table, element)
    if ((*predicate)(element, predicateArg))
      count++;
  ENDHASHTABLE
  return count;
}

// showHashTable shows the contents of a hash table, including bucket and element indexes.
// show is a function to show an element. For debugging. Uses variables defined in macro.
void showHashTable(HashTable* table, void (*show)(void*)) {
	int count = 0;
	FORHASHTABLE(table, element)
		printf("%d %d ", __i, __j);
		if (show) (*show)(element);
		count++;
	ENDHASHTABLE
	printf("showHashTable showed %d elements\n", count);
}

// addrefHashTable -- increment reference count of a HashTable

void
addrefHashTable (HashTable *table)
{
  if (! table)
    return;

  table->refcount++;
}

// releaseHashTable -- decrement reference count of table, free if count is zero

void
releaseHashTable (HashTable *table)
{
  if (! table)
    return;

  table->refcount--;
  if (! table->refcount)
    deleteHashTable (table);
}
