// DeadEnds
//
// block.h declares the structures and functions that implement the Block type.
//
// Created by Thomas Wetmore 8 March 2024.
// Last changed on 19 April 2024.

#ifndef block_h
#define block_h

#include "standard.h"

#define INITIAL_SIZE_LIST_DATA_BLOCK 30
//#define INITIAL_SIZE_LIST_DATA_BLOCK 4 // For debugging

// Block is a growable list of void* pointers.
typedef struct Block {
	int length;
	int maxLength;
	const void** elements;
} Block;

Block *createBlock(void);
void initBlock(Block*); // TODO: Find a way to make this function private to Block.
void deleteBlock(Block*, void(*d)(const void*));
void emptyBlock(Block*, void(*d)(const void*));
int lengthBlock(Block*);
bool isEmptyBlock(Block*);

const void* getBlockElement(Block*, int);
void setBlockElement(Block*, void*, void(*delete)(const void*), int);

void* findInBlock(Block*, CString, CString(*g)(const void*), int*);
void* findInSortedBlock(Block*, CString, CString(*g)(const void*), int(*c)(CString, CString), int*);

bool isInBlock(Block*, CString, CString(*g)(const void*), int*); // Linear search.
bool isInSortedBlock(Block*, CString, CString(*g)(const void*), int(*c)(CString, CString), int*); // Binary search.
void prependToBlock(Block*, const void*);
void appendToBlock(Block*, const void*);
void insertInBlock(Block*, const void*, int);
bool removeFromBlock(Block*, int, void(*d)(const void*));
bool removeFromSortedBlock(Block*, CString, CString(*g)(const void *a), int(*c)(CString, CString), void(*d)(const void*));
bool removeFromUnsortedBlock(Block*, CString, CString(*g)(const void *a), void(*d)(const void*));
bool removeFirstBlockElement(Block*, void(*d)(const void*));
bool removeLastBlockElement(Block*, void(*d)(const void*));
void sortBlock(Block*, CString(*g)(const void*), int(*c)(CString, CString));
bool isSorted(Block*, CString(*g)(const void*), int(*c)(CString, CString));

const void* getFromBlock(Block*, int);
const void* getFirstBlockElement(Block*);
const void* getLastBlockElement(Block*);

void uniqueBlock(Block*, CString(*g)(const void*), void(*d)(const void*));
Block *copyBlock(Block*, void*(*copy)(const void*));
void iterateBlock(Block*, void(*perform)(const void*));

void* searchBlock(Block* block, CString, CString(*g)(const void*), int*);
void* searchSortedBlock(Block* block, CString key, CString(*g)(const void*), int(*c)(CString, CString), int*);

// Debugging only.
void showBlock(Block*, CString(*describe)(const void*));
void fprintfBlock(FILE*, Block*, CString(*toString)(const void*));

#endif // block_h
