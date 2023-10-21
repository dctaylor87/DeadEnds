//
//  DeadEnds
//
//  validate.c -- Functions that validate Gedcom records.
//
//  Created by Thomas Wetmore on 12 April 2023.
//  Last changed on 17 October 2023.
//

#include "validate.h"
#include "gnode.h"
#include "gedcom.h"
#include "recordindex.h"
#include "lineage.h"
#include "errors.h"

bool validateDatabase(Database*);

static Database *theDatabase;

static bool debugging = true;

static bool validatePersonIndex();
static void validatePerson(GNode*);
static void validateFamily(GNode*, RecordIndex*);
static void validateSource(GNode*, RecordIndex*);
static void validateEvent(GNode*, RecordIndex*);
static void validateOther(GNode*, RecordIndex*);

static GNode *getFamily(String key, RecordIndex*);
static GNode *getPerson(String key, RecordIndex*);

int numValidations = 0;  //  DEBUG.
int abc = 4;

//  validateDatabase
//--------------------------------------------------------------------------------------------------
bool validateDatabase(Database *database)
{
	ASSERT(database);
	theDatabase = database;
	validatePersonIndex();
	//bool isOkay = true;
	//if (!validateIndex(database->personIndex)) isOkay = false;
	//if (!validateIndex(database->familyIndex)) isOkay = false;
	//if (!validateIndex(database->sourceIndex)) isOkay = false;
	//if (!validateIndex(database->eventIndex)) isOkay = false;
	//if (!validateIndex(database->otherIndex)) isOkay = false;
	//return isOkay;
	return true;
}


int xyz = 8;

//  validatePersonIndex -- Validate the person index of the current database.
//-------------------------------------------------------------------------------------------------
bool validatePersonIndex(void)
{
	FORHASHTABLE(theDatabase->personIndex, element)
		GNode* person = ((RecordIndexEl*) element)->root;
		validatePerson(person);
	ENDHASHTABLE
	return true;
}

//  validateIndex -- Validate the records in a record index. The index should include all the
//    records from a closed Gedcom file.
//-------------------------------------------------------------------------------------------------
static bool validateIndex(RecordIndex *index)
{
	//  Iterate through the records in the index.
	int bucketIndex, elementIndex;
	Word element = firstInHashTable(index, &bucketIndex, &elementIndex);
	while (element) {
		GNode* root = ((RecordIndexEl*) element)->root;
		switch (recordType(root)) {
		case GRPerson:
		case GRFamily:
		case GRSource:
		case GREvent:
		case GROther:
		case GRHeader:
		case GRTrailer:

		case GRUnknown:
			break;
		}
		element = nextInHashTable(index, &bucketIndex, &elementIndex);
	}
	return true;
}

//  validateRecordIndex -- Validate the Gedcom records in a record index. The index should
//    hold all the records from a closed Gedcom file.
//--------------------------------------------------------------------------------------------------
// void validateRecordIndex(RecordIndex *index, ErrorLog *errorLog)
// {
// 	int bucketIndex, elementIndex;
// 	Word element = firstInHashTable(index, &bucketIndex, &elementIndex);
// 	while (element) {
// 		GNode *root = ((RecordIndexEl*) element)->root;
// 		switch (recordType(root)) {
// 			case GRPerson:  validatePerson(root, index); break;
// 			case GRFamily:  validateFamily(root, index); break;
// 			case GRSource:  validateSource(root, index); break;
// 			case GREvent:   validateEvent(root, index); break;
// 			case GROther:   validateOther(root, index); break;
// 			case GRUnknown:
// 			case GRHeader:
// 			case GRTrailer: break;
// 		}
// 		element = nextInHashTable(index, &bucketIndex, &elementIndex);
// 	}
// }

extern String nameString(String);

//  validatePerson -- Validate a person record. Check all FAMC and FAMS links to families.
//--------------------------------------------------------------------------------------------------
static void validatePerson(GNode *person)
{
	GNode *nameNode = NAME(person);
	String name = "whoops";
	if (nameNode && nameNode->value) name = nameString(nameNode->value);
	if (debugging) {
		printf("---------------------------------\nvalidatePerson: Validating %s %s\n",
			   person->key, name);
	}

	//  Loop through the families the person is a child in. The person must be a child in each.
	if (debugging) printf("Doing the FAMC part.\n");

	FORFAMCS(person, family)
		if (debugging) printf("Person is a child in family %s.\n", family->key);
		int numOccurrences = 0;
		FORCHILDREN(family, child, count)
			if (debugging) {
				printf("    Child %d: %s %s\n", count, child->key, NAME(child)->value);
			}
			if (person == child) numOccurrences++;
		ENDCHILDREN
		if (numOccurrences != 1) printf("ERROR ERROR ERROR\n");
	ENDFAMCS

	//  Loop through the families the person is a spouse in. Each family should have a HUSB or
	//    WIFE node that refers back to the person.
	if (debugging) printf("Doing the FAMS part.\n");
	SexType sex = SEXV(person);
	FORFAMILIES(person, family, count) {
		if (debugging) printf("  person should be a spouse in family %s.\n", family->key);
		unused(count);
		GNode *parent = null;
		if (sex == sexMale) {
			parent = familyToHusband(family);
		} else if (sex == sexFemale) {
			parent = familyToWife(family);
		}
		ASSERT(person == parent);  // TODO: SHOULD NOT BE AS ASSERT HERE: SHOULD BE AN ERROR.
	} ENDFAMILIES

	//printf("validate person: %s: %s\n", person->gKey, name);  //  REMOVE: FOR DEBUGGING.
	//  Validate existance of NAME and SEX lines.
	//  Find all other links in the record and validate them.
}

//  validateFamily -- Validate a family node tree record. Check all HUSB, WIFE and CHIL links
//    to persons.
//--------------------------------------------------------------------------------------------------
static void validateFamily(GNode *family, RecordIndex *index)
{
	if (debugging) {
		printf("validateFamily(%s)\n", family->key);
	}
	// For each HUSB line in the family (multiples in non-traditional cases).
	FORHUSBS(family, husband, n) {
		// The husband must have one FAMS link back to this family.
		int numOccurences = 0;
		FORFAMSS(husband, fam, spouse, m) {
			numValidations++;
			if (family == fam) numOccurences++;
		} ENDFAMSS
		ASSERT(numOccurences == 1);
	} ENDHUSBS

	//  For each WIFE line in the family (multiples in non-traditional cases)...
	FORWIFES(family, wife, n) {
		int numOccurences = 0;
		FORFAMSS(wife, fam, spouse, m) {
			numValidations++;
			if (family == fam) numOccurences++;
		} ENDFAMSS
		ASSERT(numOccurences == 1);
	} ENDWIFES

	//  For each CHIL node in the family.
	FORCHILDREN(family, child, n)
		int numOccurences = 0;
		FORFAMCS(child, fam)
			numValidations++;
			if (family == fam) numOccurences++;
		ENDFAMCS
		ASSERT(numOccurences == 1);
	ENDFAMCS
	//printf("validate family: %s\n", family->gKey);
	//  Validate existance of at least one of HUSB, WIFE, CHIL.
	//  Validate that the HUSBs are male.
	//  Validate that the WIFEs are female.
	//  Validate all other links.
}

void validateSource(GNode *source, RecordIndex *index) {}

void validateEvent(GNode *event, RecordIndex *index) {}

void validateOther(GNode *other, RecordIndex *index) {}


static GNode *getFamily(String key, RecordIndex *index)
{
	GNode *root = searchRecordIndex(index, key);
	return root && recordType(root) == GRFamily ? root : null;
}

static GNode *getPerson(String key, RecordIndex *index)
{
	GNode *root = searchRecordIndex(index, key);
	return root && recordType(root) == GRPerson ? root : null;
}
