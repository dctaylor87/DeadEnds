//
//  DeadEnds Library
//

Sequence* personToChildren(GNode* person, RecordIndex*);
Sequence* personToFathers(GNode* person, RecordIndex*);
Sequence* personToMothers(GNode* person, RecordIndex*);
Sequence* familyToChildren(GNode* family, RecordIndex*);
Sequence* familyToFathers(GNode* family, RecordIndex*);
Sequence* familyToMothers(GNode* family, RecordIndex*);
Sequence* personToSpouses(GNode* person, RecordIndex*);
Sequence* personToFamilies(GNode* person, bool, RecordIndex*);
Sequence* nameToSequence(CString name, RecordIndex*, NameIndex*);
Sequence* keyToSequence(CString key, RecordIndex*);
Sequence* refnToSequence (CString value, RecordIndex*, RefnIndex*);
Sequence* stringToSequence(CString name, Database* database);

