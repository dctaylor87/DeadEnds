// DeadEnds
//
// list.h defines the types and interfaces that implement a Block-based list. Lists grow
// automatically and can be sorted or unsorted. Sorted lists require a compare function.
//
// Created by Thomas Wetmore on 22 November 2022.
// Last changed on 13 October 2024.

#ifndef list_h
#define list_h

#include "block.h"

#define enqueueList prependToList
#define dequeueList getAndRemoveLastListElement
#define pushList prependToList
#define popList getAndRemoveFirstListElement

// List defines the stucture and typedef for Lists.
typedef struct List {
	Block block; // Block of elements.
	int refCount; // how man references to the List exist
	bool sorted; // Is a sorted list.
	bool isSorted; // Is currently sorted.
	void (*delete)(void*);
	CString (*getKey)(const void*);
	int (*compare)(CString, CString);
} List;

List* createList(CString(*g)(const void*), int(*c)(CString, CString), void (*d)(void*), bool sorted);
void initList(List*, CString(*g)(const void*), int(*c)(CString, CString), void(*d)(void*), bool sorted);
void deleteList(List*);
int lengthList(List*);
void emptyList(List*);
bool isEmptyList(List*);
List* copyList(List*, void*(*c)(void*));
bool isInList(List*, CString, int*);
void* findInList(List*, CString, int* index);
// Array access.
void setListElement(List*, void*, int);
void *getListElement(List *list, int index);

void appendToList(List*, const void*);
void prependToList(List*, const void*);
void insertInList(List*, const void*, int);
bool isSortedList(List*); // Uses sorted flag.
bool isListSorted(List*); // Really checks.
void sortList(List*);
void* searchList(List* list, CString key, int* index);
void uniqueList(List*);
bool removeFromList(List*, int);
bool removeFirstListElement(List*);
bool removeLastListElement(List*);
void iterateList(List*, void(*perform)(void*));

bool isFirstElementInList(List*, void*);
bool isLastElementInList(List*, void*);

void* getFromList(List*, int);
void* getFirstListElement(List*);
void* getLastListElement(List*);

void* getAndRemoveLastListElement(List*);
void* getAndRemoveFirstListElement(List*);

Block* blockOfList(List*);
void showList(List *list, String(*describe)(void*));

extern void incrReferenceCountList (List *list, CString file, int line, CString function);

#define FORLIST(list, value)\
{\
	void* value;\
	Block* _block = &(list->block);\
	void** _values = (void**) _block->elements;\
	for (int _i = 0; _i < _block->length; _i++) {\
		value = _values[_i];\
		{
#define ENDLIST\
		}\
	}\
}

#define INCRLISTREFCOUNT(list)	incrReferenceCountList (list, __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__)

#endif
