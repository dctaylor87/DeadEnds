#ifndef _VERSION_H
#define _VERSION_H

/* This is the public build version, from configure */
#define LIFELINES_VERSION PACKAGE_VERSION

/* This is the private build version, appended to the public version */
#define LIFELINES_VERSION_EXTRA "(alpha)"

/* Function prototypes */
String get_deadends_version (INT maxlen);
void print_version (CString program);

#endif /* _VERSION_H */
