//
// DeadEnds
// sort.h -- Lists are kept sorted if a compare function is provided when they are created.
// Sort.c provides a sort function on Lists using quick sort, and a search function that uses
// binary search.
//
// Created by Thomas Wetmore on 21 November 2022.
// Last changed on 22 March 2024.

#ifndef sort_h
#define sort_h

#include "standard.h"

void sortElements(void**, int, CString(*g)(void*), int(*c)(CString, CString));
void* linearSearch(void**, int, CString, CString(*)(const void*), int*);
void* binarySearch(void**, int, CString, CString(*)(const void*), int(*c)(CString, CString), int*);

void insertAtIndex(void**, int len, const void*, int index);

#endif // sort_h
