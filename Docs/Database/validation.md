# Validation Stack
This section describes the process that validates the contents of *DeadEnds* databases. Databases are created by the *read stack* described in ADD A LINK.

If no errors were found in the Gedcom syntax, or in the other single record checks done by the *read stack*, there will be a database of all the records from the file. These records must be further validated, because no inter-record checks were done by the *read stack*. The database must be *closed* &mdash; all families that persons refer to, and all persons that families refer to, must exist in the database. When a person has a FAMC link to a family he or she is a child in, that family must also have a single CHIL link back to the person. Likewise for spouses.

## Validation Stack
### bool validateDatabase(Database *database, ErrorLog *errorLog)
*validateDatabase* is the top layer of the validation stack. It is passed in the database to validate and the error log to post any errors to. It calls the functions that specifically validate differnt regions of the database. These functions are *validatePersonIndex*, *validateFamilyIndex*, *validateSourceIndex*, *validateEventIndex* and *validateOtherIndex*.
### bool validatePersonIndex(void)
The signature needs to be put in final shape. It calls *validatePerson*.
### static void validatePerson(GNode *person)
{
	FORHASHTABLE(theDatabase->personIndex, element)
		GNode* person = ((RecordIndexEl*) element)->root;
		validatePerson(person);
	ENDHASHTABLE
	return true;
}

//  validateFamilyIndex -- Validate the family index of the current database.
//-------------------------------------------------------------------------------------------------
bool validateFamilyIndex(void)
{
	FORHASHTABLE(theDatabase->familyIndex, element)
		GNode *family = ((RecordIndexEl*) element)->root;
		validateFamily(family);
	ENDHASHTABLE
	return true;
}

extern String nameString(String);

//  validatePerson -- Validate a person record. Check all FAMC and FAMS links to families.
//--------------------------------------------------------------------------------------------------
static void validatePerson(GNode *person)
{
	if (debugging) { printf("Validating %s %s\n", person->key, NAME(person)->value); }

	//  Loop through the families the person is a child in.
	FORFAMCS(person, family)
		//if (debugging) printf("Person is a child in family %s.\n", family->key);
		int numOccurrences = 0;
		FORCHILDREN(family, child, count)
			//if (debugging) { printf("    Child %d: %s %s\n", count, child->key, NAME(child)->value); }
			if (person == child) numOccurrences++;
		ENDCHILDREN
		if (numOccurrences != 1) printf("ERROR ERROR ERROR\n");
	ENDFAMCS

	//  Loop through the families the person is a spouse in.
	//if (debugging) printf("Doing the FAMS part.\n");
	SexType sex = SEXV(person);
	FORFAMILIES(person, family) {
		if (debugging) printf("  person should be a spouse in family %s.\n", family->key);
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
static void validateFamily(GNode *family)
{
	if (debugging) {
		printf("validateFamily(%s)\n", family->key);
	}
	// For each HUSB line in the family (multiples in non-traditional cases).
	FORHUSBS(family, husband)
		// The husband must have one FAMS link back to this family.
		int numOccurences = 0;
		FORFAMSS(husband, fam)
			numValidations++;
			if (family == fam) numOccurences++;
		ENDFAMSS
		ASSERT(numOccurences == 1);
	ENDHUSBS

	//  For each WIFE line in the family (multiples in non-traditional cases)...
	FORWIFES(family, wife, n) {
		int numOccurences = 0;
		FORFAMSS(wife, fam) {
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