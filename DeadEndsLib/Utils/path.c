// DeadEnds
//
// path.c has functions to manipulate UNIX file paths.
//
// Created by Thomas Wetmore on 14 December 2022.
// Last changed on 2 September 2025.

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#include "standard.h"
#include "de-strings.h"
#include "path.h"

#define MAXPATHBUFFER 4096

static bool isPathSeparator (char c);

// resolveFile tries to find a file within a sequence of paths.
CString oldResolveFile(CString name, CString path) {
    if (!name || *name == 0) return null;
    if (!path || *path == 0) return strsave(name);
    if (strchr(name, '/') != null) return strsave(name);
    char buf1[MAXPATHBUFFER];
    strcpy(buf1, path);
    char buf2[MAXPATHBUFFER];
    char* p = strtok(buf1, ":");
    while (p) {
        snprintf(buf2, sizeof(buf2), "%s/%s", p, name);
        if (access(buf2, F_OK) == 0) return strsave(buf2);
        p = strtok(null, ":");
    }
    return null;
}

/// resolveFile tries to find a file within a colon-separated path list.
/// If not found, and a suffix is provided, tries again with the suffix appended.
CString resolveFile(CString name, CString path, CString suffix) {

    if (!name || *name == 0) return null;  // No file name.
    if (!path || *path == 0) return strsave(name);  // No path.
    if (strchr(name, '/')) return strsave(name);  // File name is absolute.
    char buf1[MAXPATHBUFFER];
    char fullPath[MAXPATHBUFFER];

    // Search path list for original name
    strcpy(buf1, path);
    for (char* dir = strtok(buf1, ":"); dir; dir = strtok(NULL, ":")) {
        snprintf(fullPath, sizeof(fullPath), "%s/%s", dir, name);
        if (access(fullPath, F_OK) == 0) return strsave(fullPath);
    }
    // Try with suffix if provided
    if (suffix && *suffix != 0) {
        strcpy(buf1, path);  // Reset buf1 because strtok modifies it
        String fmt = (*suffix == '.') ? "%s/%s%s" : "%s/%s.%s";
        for (char* dir = strtok(buf1, ":"); dir; dir = strtok(NULL, ":")) {
            snprintf(fullPath, sizeof(fullPath), fmt, dir, name, suffix);
            if (access(fullPath, F_OK) == 0) return strsave(fullPath);
        }
    }
    return null;
}

// fopenPath attempta to open a file using a path variable.
FILE *fopenPath(CString name, CString mode, CString path, CString suffix) {
    CString str;
    if (!(str = resolveFile(name, path, suffix))) return null;
    FILE *file = fopen(str, mode);
    stdfree(str);
    return file;
}

// lastPathSegment returns the last componenet of a path. Returns static memory.
String oldLastPathSegment (String path) {
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
// lastPathSegment returns the last componenet of a path. Returns static memory.
String lastPathSegment (CString path) {
    char *lastSlash = strrchr (path, '/');
    if (lastSlash)
        return (lastSlash + 1);
    else
        return path;
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

/* expandSpecialFilenameChars -- Replace ~ with home */

static String getUserHomedir (CString username);
static String getHome (void);

bool
expandSpecialFilenameChars (String buffer, int buflen, int utf8)
{
  char * sep=0;
  if (buffer[0]=='~') {
    if (isDirSep(buffer[1])) {
      String home = getHome();
      if (home && home[0]) {
	String tmp;
	if ((int)strlen(home) + 1 + (int)strlen(buffer) > buflen) {
	  return false;
	}
	tmp = strsave(buffer);
	buffer[0] = 0;
	destrapps(buffer, buflen, utf8, home);
	destrapps(buffer, buflen, utf8, tmp+1);
	stdfree(tmp);
	tmp = 0;
	return true;
      }
    }
    /* check for ~name/... and resolve the ~name */
    if ((sep = strchr(buffer,DECHRDIRSEPARATOR))) {
      String username = strsave(buffer+1);
      String homedir;
      username[sep-buffer+1] = 0;
      homedir = getUserHomedir(username);
      stdfree(username);
      username = 0;
      if (homedir) {
	String tmp=0;
	if ((int)strlen(homedir) + 1 + (int)strlen(sep+1) > buflen) {
	  return false;
	}
	tmp = strsave(sep+1);
	buffer[0] = 0;
	destrapps(buffer, buflen, utf8, homedir);
	destrapps(buffer, buflen, utf8, tmp+(sep-buffer+1));
	stdfree(tmp);
	tmp = 0;
	return true;
      }
    }
  }
  return true;
}

/* getHome -- Find user's home directory. */

static String getHome (void)
{
  String home;
#ifdef WIN32
  /* replace ~ with user's home directory, if present */
  /* TODO: Or HOMEPATH, HOMESHARE, or USERPROFILE ? */
  home = (String)getenv("APPDATA");
#else
  home = (String)getenv("HOME");
#endif
  return home;
}

/* getUserHomedir -- Return home directory of specified user
   returns 0 if unknown or error
   returns alloc'd value. */
 
static String
getUserHomedir (CString username)
{
  struct passwd *pw=0;
  if (! username)
    return 0;
#ifdef WIN32
  /* This could be implemented for NT+ class using NetUserGetInfo,
     but I doubt it's worth the trouble. Perry, 2005-11-25.  */
#else /* not WIN32 */
  setpwent();
  /* loop through the password file/database
     to see if the string following ~ matches
     a login name.  */
  while ((pw = getpwent())) {
    if (eqstr(pw->pw_name, username)) {
      /* found user in passwd file */
      String homedir = strsave(pw->pw_dir);
      endpwent();
      return homedir;
    }
  }
  endpwent();
#endif
  return 0;
}
