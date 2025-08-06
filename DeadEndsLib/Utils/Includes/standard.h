// DeadEnds
//
// standard.h defines useful things.
//
// Created by Thomas Wetmore on 1 November 2022.
// Last changed on 16 November 2024.

#ifndef standard_h
#define standard_h

typedef char* String;
typedef const char* CString;

#include <sys/types.h>
#include <ansidecl.h> // ATTRIBUTE_UNUSED
#include <stdint.h>   // uint64_t and friends
#include <stdlib.h> // malloc, free, abort.
#include <stdio.h>  // FILE, fopen, fclose, printf, sprintf, fprintf.
#include <string.h> // strlen, strcmp, strcpy, strcmp, strrchr.
#include <ctype.h>
#include <stdbool.h> // bool, true and false.
#include <unistd.h>
#include "path.h"

#define DEBUGALLOCS // EMPTY
#define MAXSTRINGSIZE 512
#define DeadEndsNAN  __INT_MAX__  // Poor man's implementation of NAN for integers.

// CharacterType -- Characters are partitioned into different types.
typedef enum { Letter = 300, Digit, White, Other } CharType;

// Catenate two strings and return a new String on the heap with the value.
extern String strconcat(String s1, String s2);
String strsave(CString);  // Save String in heap.
String strnsave(CString, int);  // Save String prefix in heap.
bool iswhite(int);       // Is character white space?
bool allwhite(String);   // Is String all white space?
void striptrail(String);  // Strip trailing white space.
int chartype(int);      // Return type of a character.
void alloc_out(String); // standard.c
CharType characterType(int);  // standard.c
String lower(String);  // Convert a string to lower case.
String upper(String);  // Convert a string to upper case.
String capitalize(String);

void basicDelete(void*);

#define MAXLINELEN 4096  // Max length for Gedcom lines when reading files.

#define unused(x) (void)(x)


// Max and min macros.
#ifndef max
#define max(x,y) ((x)>(y)?(x):(y))
#endif

#ifndef min
#define min(x,y) ((x)>(y)?(y):(x))
#endif

// User interface to the standard functions.
void* _alloc(size_t, CString, int) __attribute__ ((__malloc__)) __attribute__ ((alloc_size (1))) __attribute__ ((assume_aligned (8))) __attribute__ ((__returns_nonnull__));
void _free(void* ptr, String, int);
void* _realloc(void* ptr, size_t, CString, int) __attribute__ ((__malloc__)) __attribute__ ((alloc_size (2))) __attribute__ ((assume_aligned (8))) __attribute__ ((__returns_nonnull__));
bool isLetter(int);  // Is character is an Ascii letter?
String trim(String, int); // Trim String to size.
void _logAllocations(bool);  // Turn allocation logging on and off.
void logRefCountChange (void *element, CString eltName, int refCount,
			CString file, int line, CString function);

// Turn on alloc and free debugging.
#ifdef DEBUGALLOCS // Debugging allocs and free.
	#define stdalloc(l)       _alloc(l, __FILE__, __LINE__)
	#define stdfree(p)        _free((void *)p, __FILE__, __LINE__)
	#define stdrealloc(p, s)  _realloc((void *)p, s, __FILE__, __LINE__)
	#define logAllocations(b) _logAllocations((b))
#else // Not debugging allocs and frees.
	#define stdalloc(l)        malloc((void *)(l))
	#define stdfree(p)         free(((void *)p))
	#define stdrealloc(p, s)   reallloc ((void *)p, s)
	#define logAllocations(b)
#endif

#define fatal(msg)    _fatal(__FILE__, __LINE__, msg, __PRETTY_FUNCTION__)
#define FATAL()       _fatal(__FILE__, __LINE__, NULL, __PRETTY_FUNCTION__)
#define ASSERT(expr)  do { if(!(expr)) _fatal(__FILE__, __LINE__, "ASSERT FAILED: " #expr, __PRETTY_FUNCTION__); } while (0)
#define eqstr(s,t)    (!strcmp((s),(t)))
#define nestr(s,t)    (strcmp((s),(t)))

#define ARRAYSIZE(a)	(sizeof(a)/sizeof(a[0]))

void _fatal(CString, int, CString, CString) __attribute__ ((__noreturn__)); // standard.c
void _assert(bool, String, int); // standard.c

extern CString version;		// standard.c

// These macros are used for character types in the chartype() function.
#define WHITE  ' '
#define LETTER 'a'
#define DIGIT  '0'
#define ZERO    0

#define null ((void*) 0)

// Debugging aids.
#define PH if(debugging) fprintf(stderr, "%s %s %d\n", __FUNCTION__,\
	lastPathSegment(__FILE__), __LINE__);

#endif // standard_h
