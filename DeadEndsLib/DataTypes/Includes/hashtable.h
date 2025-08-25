//
//  DeadEnds Library
//
//  hashtable.h is the header file for the HashTable data type. A HashTable is an array of buckets.
//  A bucket is a list of elements with the same hash value. Elements are defined by the user.
//  The user a getKey function to return the key of an element, and a compare function that is
//  use to keep the elements in a Bucket sorted.
//
//  Created by Thomas Wetmore 29 November 2022.
//  Last changed on 23 August 2025.
//

#ifndef hashtable_h
#define hashtable_h

#include "standard.h"
#include "block.h"

#define INITIAL_BUCKET_LENGTH 30
#define SORT_THRESHOLD 30

//#define INITIAL_BUCKET_LENGTH 4  // DBUG: Make the initial bucket length small for debugging.
//#define SORT_THRESHOLD 5  //  DBUG: Make the sort threshold small to debug the quick sort.

// Bucket is the type of HashTable buckets.
typedef struct Bucket {
	Block block;
} Bucket;

// HashTable is the type that implements a hash table. The getKey, compare and delete functions
// customize the elements used in specific HashTables. getKey gets the key of an element;
// compare compares two keys; and delete deletes an element.
typedef struct HashTable {
	int refCount;	//  Number of references to the table.
	int numBuckets; // Should be a prime number.
	CString (*getKey)(const void*);
	int (*compare)(CString, CString);
	void (*delete)(void*);
	Bucket** buckets;
} HashTable;

// User interface to HashTable.
HashTable* createHashTable(CString(*)(const void*), int(*)(CString, CString), void(*)(void*), int);
void deleteHashTable(HashTable*);
bool isInHashTable(HashTable*, CString key);
void* detailSearchHashTable(HashTable* table, CString key, int* phash, int* pindex);
void* searchHashTable(HashTable*, CString key);
void* searchHashTableWithElement(HashTable* table, void* element);

void addToHashTable(HashTable*, void*, bool);
bool addToHashTableIfNew(HashTable*, void*);
void *firstInHashTable(HashTable*, int*, int*);
void* nextInHashTable(HashTable*, int*, int*);
void *lastInHashTable(HashTable*, int*, int*);
void* previousInHashTable(HashTable*, int*, int*);

int sizeHashTable(HashTable*);
void showHashTable(HashTable*, void(*show)(void*));
void dumpHashTable(HashTable*, void(*show)(void*));
/*static*/ int getHash(CString, int);
void removeFromHashTable(HashTable*, CString key);
void iterateHashTable(HashTable*, void (*)(void*));
int iterateHashTableWithPredicate(HashTable*, bool(*)(void*));
int iterateHashTableWithPredicate2(HashTable*, void *, bool(*)(void*, void*));

extern void addrefHashTable (HashTable *table); // increment ref. count of table
extern void releaseHashTable (HashTable *table); // decrement ref. count of table, free if zero

extern Bucket *createBucket (void);
extern int lengthBucket (Bucket* bucket);
void appendToBucket(Bucket*, void* element);
void removeElement(HashTable*, void* element);
extern void incrReferenceCountTable (HashTable *table, CString file, int line, CString function);

// FORHASHTABLE and ENDHASHTABLE iterate the elements in a HashTable.
#define FORHASHTABLE(table, element) {\
		int __i = 0, __j = 0;\
		HashTable *__table = table;\
		void *element = null;\
		void *__element = firstInHashTable(__table, &__i, &__j);\
		for(; __element; __element = nextInHashTable(__table, &__i, &__j)) {\
			element = __element;
#define ENDHASHTABLE }}

#define INCRTABLEREFCOUNT(table)	incrReferenceCountTable (table, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__)

#endif // hashtable_h
