// DeadEnds
//
// generatekey.h
//
// Created by Thomas Wetmore on 20 July 2024.
// Last changed on 14 March 2025.

#ifndef generatekey_h
#define generatekey_h

// Public interface.
// generateRecordKey generates a new random key for a record type.
String generateRecordKey(RecordType);
extern bool keyInPreviousFiles(CString key);
extern bool keyInCurrentFile(CString key);
extern void processKey(String key, bool defining);

#endif // generatekey.h
