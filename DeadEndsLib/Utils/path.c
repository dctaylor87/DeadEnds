// DeadEnds
//
// path.c has functions to manipulate UNIX file paths.
//
// Created by Thomas Wetmore on 14 December 2022.
// Last changed on 26 July                 2024.

#include <unistd.h>
#include "standard.h"
#include "path.h"

#define MAXPATHBUFFER 1024

// filePath finds a file in a sequence of paths.
CString resolveFile(CString name, CString path) {
	if (!name || *name == 0) return null;
	if (!path || *path == 0) return name;
	if (*name == '/' || *name == '.') return name; // Bug: . could be part of name.
	if (strlen(name) + strlen(path) >= MAXPATHBUFFER) return null;
	unsigned char buf1[MAXPATHBUFFER];
	strcpy((String) buf1, path);
	String p = (String) buf1; // Convert :'s to 0's.
	int c;
	while ((c = *p)) {
		if (c == ':') *p = 0;
		p++;
	}
	*(++p) = 0; // Extra 0.
	p = (String) buf1;
	unsigned char buf2[MAXPATHBUFFER]; // Full file names.
	String q;
	while (*p) {
		q = (String) buf2;
		strcpy(q, p);
		q += strlen(q);
		strcpy(q++, "/");
		strcpy(q, name);
		if (access((const char*) buf2, 0) == 0)
			return strsave((String) buf2); // Memory leak.
		p += strlen(p);
		p++;
	}
	return name;
}

// fopenPath attempta to open a file using a path variable.
FILE *fopenPath(CString name, CString mode, CString path) {
	CString str;
	if (!(str = resolveFile(name, path))) return null;
	return fopen(str, mode);
}

// lastPathSegment returns the last componenet of a path. Returns static memory.
String lastPathSegment (CString path) {
	static unsigned char scratch[MAXPATHBUFFER];
	if (!path || *path == 0) return NULL;
	int len = (int) strlen(path);
	String p = (String) scratch, q;
	strcpy(p, path);
	if (p[len-1] == '/') {
		len--;
		p[len] = 0;
	}
	q = p;
	int c;
	while ((c = *p++)) {
		if (c == '/') q = p;
	}
	return q;
}

/* pathMatch -- are paths the same?
   Handles WIN32 filename case insensitivity.  */

bool
pathMatch (CString path1, CString path2)
{
#ifdef WIN32
	return !stricmp(path1, path2);
#else
	return !strcmp(path1, path2);
#endif
}

/* pathConcatAllocate -- add file & directory together into newly alloc'd
   string & return.

   dir:  [IN]  directory (may be NULL)
   file: [IN]  file (may be NULL)   */

String
pathConcatAllocate (CString dir, CString file)
{
  int len = (dir ? strlen(dir) : 0) + (file ? strlen(file) : 0) +2;
  String buffer = stdalloc(len);
  int myutf8=0; /* buffer is big enough, so won't matter */
  return pathConcat (dir, file, myutf8, buffer, len);
}

/* pathConcat -- add file & directory together
   handles trailing / in dir and/or leading / in file
   returns no trailing / if file is NULL.

   dir:  [IN]  directory (may be NULL)
   file: [IN]  file (may be NULL)  */

String
pathConcat (CString dir, CString file, int utf8, String buffer, int buflen)
{
  ASSERT(buflen);
  buffer[0] = 0;
  if (dir && dir[0]) {
    destrapps(buffer, buflen, utf8, dir);
    if (isDirSep(buffer[strlen(buffer)-1])) {
      /* dir ends in sep */
      if (!file || !file[0]) {
	/* dir but no file, we don't include trailing slash */
	buffer[strlen(buffer)-1] = 0;
      } else {
	if (isDirSep(file[0])) {
	  /* file starts in sep */
	  destrapps(buffer, buflen, utf8, &file[1]);
	} else {
	  /* file doesn't start in sep */
	  destrapps(buffer, buflen, utf8, file);
	}
      }
    } else {
      /* dir doesn't end in sep */
      if (!file || !file[0]) {
	/* dir but no file, we don't include trailing slash */
      } else {
	if (isDirSep(file[0])) {
	  /* file starts in sep */
	  destrapps(buffer, buflen, utf8, file);
	} else {
	  /* file doesn't start in sep */
	  destrapps(buffer, buflen, utf8, DESTRDIRSEPARATOR);
	  destrapps(buffer, buflen, utf8, file);
	}
      }
    }
  } else {
    /* no dir, include file exactly as it is */
    if (file && file[0])
      destrapps(buffer, buflen, utf8, file);
  }

  return buffer;
}

/* isDirSep -- Is directory separator character ?
   handles WIN32 characters.  */

bool
isDirSep (char c)
{
#ifdef WIN32
	return c=U21=DECHRDIRSEPARATOR || c=='/';
#else
	return c==DECHRDIRSEPARATOR;
#endif
}
