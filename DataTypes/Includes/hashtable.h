//
// DeadEnds
//
// hashtable.h -- Implements a HashTable. A HashTable is an array of MAX_HASH buckets. A bucket
// is a list of elements with the same hash value. Elements are defined by the user. The user
// must provide a compare function used to keep the elements in the buckets sorted.
//
//  Created by Thomas Wetmore 29 November 2022.
//  Last changed on 13 April 2024.
//

#ifndef hashtable_h
#define hashtable_h

#include "standard.h"
#include "block.h"

//#define INITIAL_BUCKET_LENGTH 30
#define INITIAL_BUCKET_LENGTH 4  // DBUG: Make the initial bucket length small for debugging.
//#define SORT_THRESHOLD 30
#define SORT_THRESHOLD 5  //  DBUG: Make the sort threshold small to debug the quick sort.

// Bucket is the type of HashTable buckets.
typedef struct Bucket {
	Block block;
} Bucket;

// HashTable is the type that implements a hash table. The getKey, compare and delete functions
// customize the elements used in a specific HashTables. getKey gets the key of an element;
// compare compares to keys; and delete deletes an element.
typedef struct HashTable {
	int refcount;	//  Number of references to the table.
	int numBuckets; // Should be a prime number.
	CString (*getKey)(void*);
	int (*compare)(CString, CString);
	void (*delete)(void*);
	Bucket** buckets;
} HashTable;

// User interface to HashTable.
HashTable* createHashTable(CString(*g)(void*), int(*c)(CString, CString), void(*d)(void*), int numBuckets);
void deleteHashTable(HashTable*);
bool isInHashTable(HashTable*, CString key);
void* detailSearchHashTable(HashTable* table, CString key, int* phash, int* pindex);
void* searchHashTable(HashTable*, CString key);
void* searchHashTableWithElement(HashTable* table, void* element);

void addToHashTable(HashTable*, void*, bool);
bool addToHashTableIfNew(HashTable*, void*);
void *firstInHashTable(HashTable*, int*, int*);
void* nextInHashTable(HashTable*, int*, int*);

int sizeHashTable(HashTable*);
void showHashTable(HashTable*, void(*show)(void*));
/*static*/ int getHash(CString, int);
void removeFromHashTable(HashTable*, CString key);
int iterateHashTableWithPredicate(HashTable*, bool(*)(void*));

extern void addrefHashTable (HashTable *table); // increment ref. count of table
extern void releaseHashTable (HashTable *table); // decrement ref. count of table, free if zero

//  SHOULDN'T THE BUCKET FUNCTIONS BE STATIC, SO NOT DECLARED IN HERE AT ALL??
Bucket *createBucket(void);
int lengthBucket(Bucket*);
void deleteBucket(Bucket*, void(*d)(void*));
//void* searchBucket(Bucket*, String key, int(*c)(void*, void*), int* index);  // Search a bucket.

void appendToBucket(Bucket*, void* element);
void removeElement(HashTable*, void* element);

// FORHASHTABLE and ENDHASHTABLE are macros that iterate over the elements in a HashTable.
#define FORHASHTABLE(table, element) {\
			int __i = 0, __j = 0;\
			HashTable *__table = table;\
			void *element = null;\
			void *__element = firstInHashTable(__table, &__i, &__j);\
			while (__element) {{\
				element = __element;

#define ENDHASHTABLE }\
			__element = nextInHashTable(__table, &__i, &__j);\
		}}\


#define NEWFORHASHTABLE(table, element) {\
			int __i = 0, __j = 0;\
			HashTable *__table = table;\
			void *element = null;\
			void *__element = firstInHashTable(__table, &__i, &__j);\
			for(; __element; __element = nextInHashTable(__table, &__i, &__j)) {\
				element = __element;

#define NEWENDHASHTABLE \
				__element = nextInHashTable(__table, &__i, &__j);\
}}\


#endif // hashtable_h
