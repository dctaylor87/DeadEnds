# gedcom.c

|Component|Description|
|:---|:---|
|RecordType recordType(GNode *root)|Return the type of a Gedcom record (GNode tree).|
|int compareRecordKeys(Word p, Word q)|Compare record keys, doing tricks if they are numeric, and might not work right.|