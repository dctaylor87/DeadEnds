// DeadEnds
//
// path.c has functions to manipulate UNIX file paths.
//
// Created by Thomas Wetmore on 14 December 2022.
// Last changed on 26 July                 2024.

#include <unistd.h>
#include "standard.h"
#include "de-strings.h"
#include "path.h"

#define MAXPATHBUFFER 1024

static bool isPathSeparator (char c);

// filePath finds a file in a sequence of paths.
CString resolveFile(CString name, CString path) {
	if (!name || *name == 0) return null;
	if (!path || *path == 0) return name;
	if (*name == '/' || *name == '.') return name; // Bug: . could be part of name.
	if (strlen(name) + strlen(path) >= MAXPATHBUFFER) return null;
	unsigned char buf1[strlen(path) + 2];
	strcpy((String) buf1, path);
	String p = (String) buf1; // Convert :'s to 0's.
	int c;
	while ((c = *p)) {
		if (c == ':') *p = 0;
		p++;
	}
	*(++p) = 0; // Extra 0.
	p = (String) buf1;
	unsigned char buf2[strlen(path) + strlen(name) + 2]; // Full file names.
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
	return c==DECHRDIRSEPARATOR || c=='/';
#else
	return c==DECHRDIRSEPARATOR;
#endif
}

/* compressPath -- return path truncated

   Returns the trailing MIN(len, 120) bytes of path.  If path is too
   long, it is truncated with the start replaced with '...' .

   The return is in a static buffer.
   Created 2001/12/22 for LifeLines by Perry Rapp. */

String
compressPath (CString path, int len)
{
  static char buf[120];
  int pathlen = strlen(path);

  if (len > (int)sizeof(buf)-1)
    len = (int)sizeof(buf)-1;
  /* TODO: be nice to expand "."  */
  if (pathlen > len) {
    String dotdotdot = "...";
    int delta = pathlen - len - strlen(dotdotdot);
    strcpy(buf, dotdotdot);
    strcpy(buf+strlen(dotdotdot), path+delta);
  } else
    strcpy(buf, path);

  return buf;
}

/* chopPath -- copy path into dirs, & zero-separate all dirs
   path:  [IN]  path list to copy
   dirs:  [OUT] output buffer

   We replace the path separators with NUL and add an addition NUL at
   the end.

   NB: dirs should be two bytes larger than strlen(path)
       ignore zero length paths */
int
chopPath (CString path, String dirs)
{
  int ndirs = 0;
  String p;
  CString q;
  char c=0;

  p = dirs;;
  q = path;
  while ((c = *q)) {
    if (isPathSeparator(c)) {
      if (p == dirs || p[-1] == 0) {
	q++;
      } else {
	*p++ = 0;
	q++;
	++ndirs;
      }
    } else {
      *p++ = *q++;
    }
  }
  if (!(p == dirs || p[-1] == 0)) {
    *p++ = 0;
    ++ndirs;
  }
  *p = 0; /* ends with extra trailing zero after last one */
  return ndirs;
}

/* isPathSeparator -- Is 'c' a path separator character
   handle WIN32 characters */

static bool
isPathSeparator (char c)
{
  // : on UNIX and ; on Windows
  return (c == DECHRPATHSEPARATOR);
}
