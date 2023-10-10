# lineage.c

|Component|Description|
|:---|:---|
|extern Database *theDatabase|The database.|
|GNode* personToFather(GNode* node)|Return the father of a person.|
|GNode* personToMother(GNode* node)|Return the mother of a person.|
|GNode* personToPreviousSibling(GNode* indi)|Return the previous sibling of a person.|
|GNode* personToNextSibling(GNode* indi)|Return the next sibling of a person.|
|GNode* familyToHusband(GNode* node)|Return the first husband of a family.|
|GNode* familyToWife(GNode* node)|Return the first wife of a family.|
|GNode* familyToFirstChild(GNode* node)|Return the first child of a family.|
|GNode* familyToLastChild(GNode* node)|Return the last child of a family.|
|int numberOfSpouses(GNode* person)|Return the number of spouses of a person.|
|int numberOfFamilies(GNode* person)|Return the number of families a person is a spouse in.|
|GNode* personToFamilyAsChild(GNode* person)|Return the first family a person is in as a child.|
|String personToName(GNode* person, int length)|Return the name of a person.|
|String personToTitle(GNode* indi, int len)|Return the first title of a person.|