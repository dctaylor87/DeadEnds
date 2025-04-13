// DeadEnds
//
// set.h is the header file for the Set type.
//
// Created by Thomas Wetmore on 22 November 2022.
// Last changed on 15 June 2024.

#ifndef set_h
#define set_h

#include "list.h"

// Set implements a set with a sorted List. Its elements point to structures with String keys.
// The getkey and compare functions are used to extract and compare keys.
typedef struct Set {
	List list;
} Set;

// Public interface.
Set* createSet(CString(*get)(const void*), int(*cmp)(CString, CString), void(*del)(void*));
void deleteSet(Set*);
int lengthSet(Set*);
bool isInSet(Set*, CString);
void addToSet(Set*, const void*);
void removeFromSet(Set*, CString);
void iterateSet(Set*, void(*iter)(void*));
void showSet(Set*, String(*show)(void*));
List* listOfSet(Set*); // Underlying List.

// FORSET and ENDSET are macros that iterate the elements of a Set.
#define FORSET(set, element)\
{\
	void* element;\
	List* _list = &(set->list);\
	Block* _block = &(_list->block);\
	void** _elements = (void**) _block->elements;\
	for (int _i = 0; _i < _block->length; _i++) {\
		element = _elements[_i];\
		{

#define ENDSET\
		}\
	}\
}

#endif // set_h
