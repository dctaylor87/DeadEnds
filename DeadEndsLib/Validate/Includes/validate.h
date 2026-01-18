//
//  DeadEnds Library
//
//  validate.h
//
//  Created by Thomas Wetmore on 12 April 2023.
//  Last changed on 4 June 2025.
//

#ifndef validate_h
#define validate_h

typedef struct GNode GNode;
typedef struct HashTable HashTable;
typedef HashTable IntegerTable;
typedef HashTable RecordIndex;
typedef HashTable RefnIndex;

typedef enum ValidationCodes {
	VCclosedKeys = 1,
	VClineageLinking = 2,
	VCnamesAndSex = 4,
} ValidationCodes;

extern void validatePersons(RecordIndex*, CString name, IntegerTable*, ErrorLog*);
extern void validateFamilies(RecordIndex*, CString name, IntegerTable*, ErrorLog*);
extern bool validateRecord (GNode *, Database *, ErrorLog *);
extern RefnIndex* getReferenceIndex(RecordIndex*, CString name, IntegerTable*, ErrorLog*);
extern bool validateSourceIndex(Database* database, ErrorLog* errorLog);
extern bool validateEventIndex(Database* database, ErrorLog* errorLog);
extern bool validateOtherIndex(Database* database, ErrorLog* errorLog);
extern int rootLine(GNode*, IntegerTable*);

/* valid.c */
//extern bool pointer_value(String);
extern bool validateNewPerson(GNode *new, GNode *old,
			      Database *database, ErrorLog *errorLog);
extern bool validateNewFamily(GNode *new, GNode *old,
			      Database *database, ErrorLog *errorLog);
//extern bool valid_name(String);
extern bool validateNewRecord(GNode *newNode, GNode *origNode, RecordType ntype,
			      Database *database, ErrorLog *errorLog);
extern bool validateNewSource(GNode *new, GNode *orig,
			      Database *database, ErrorLog *errorLog);
extern bool validateNewEvent(GNode *new, GNode *orig,
			     Database *database, ErrorLog *errorLog);
extern bool validateNewOther(GNode *new, GNode *orig,
			     Database *database, ErrorLog *errorLog);

//#if 0
/* write.c */

/* XXX interface and/or name might change XXX */
extern GNode *file_to_node (String fname, String *pmsg, bool *pemp);
//#endif
#endif // validate_h
