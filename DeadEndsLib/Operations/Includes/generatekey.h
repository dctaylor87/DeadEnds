// DeadEnds
//
// generatekey.h
//
// Created by Thomas Wetmore on 20 July 2024.
// Last changed on 20 July 2024.

#ifndef generatekey_h
#define generatekey_h

void initRecordKeyGenerator(void);
String generateRecordKey(RecordType);
extern bool keyInPreviousFiles(CString key);
extern bool keyInCurrentFile(CString key);
extern void processKey(String key, bool defining);

#endif /* generatekey_h */
