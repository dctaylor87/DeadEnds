//
//  DeadEnds
//
//  import.c -- Read Gedcom files and build a database from them.
//
//  Created by Thomas Wetmore on 13 November 2022.
//  Last changed on 10 October 2023.
//

#include "standard.h"
#include "import.h"
#include "gnode.h"
#include "stringtable.h"
#include "recordindex.h"
#include "gedcom.h"
#include "splitjoin.h"
#include "database.h"
#include "validate.h"
#include "errors.h"

String currentGedcomFileName = null;
int currentGedcomLineNumber = 1;

Database *theDatabase = null;

extern bool validateIndex(RecordIndex *index);
static String updateKeyMap(GNode *root, StringTable* keyMap);
static void rekeyDatabase(Database*, StringTable *keyMap);
static void rekeyIndex(RecordIndex*, StringTable *keyMap);
static void outputErrorLog(ErrorLog* errorLog);
static void setupDatabase(List *recordIndexes);
static void addIndexToDatabase(RecordIndex *index, Database *database);

bool importFromFile(FILE*, RecordIndex*, ErrorLog*);

extern StringTable *keyMap;  //  Map between original record keys and actual keys.

// Error messages defined elsewhere.
extern String idgedf, gdcker, gdnadd, dboldk, dbnewk, dbodel, cfoldk, dbdelk, dbrdon;

static GNode *normalizeNodeTree (GNode*);

static bool debugging = true;

//  Counters for the record types with keys.
//--------------------------------------------------------------------------------------------------
static int numPersons = 0;
static int numFamilies = 0;
static int numSources = 0;
static int numEvents = 0;
static int numOthers = 0;

//  The steps for each file are:
//    Create a record index.
//    Create a key remap table.
//    For each Gedcom record in the file...
//      Create a new key for the record, add it to the rekey table and update the record count.
//      Change the key in the root to the new key. (Don't change other keys yet.))
//    After all records are processed process each record in the index and use the remap table
//      to remap all the key values.
//
//  After all files are processed, and there are no errors, create the database from the indexes.
//  Remove the record indexes and remapping tables.
//

//  importFromFiles -- Import Gedcom files into a database.
//--------------------------------------------------------------------------------------------------
bool importFromFiles(String fileNames[], int count, ErrorLog *errorLog)
//  filesNames -- Names of the files to import.
//  count -- Number of files to import.
//  errorLog -- Error log.
{
    //  Create a list for file record indexes, one for each file that can be opened.
    List *listOfIndexes = createList(null, null, null);  //  TODO: Supply the delete function.

    //  Loop through the file names.
    for (int i = 0; i < count; i++) {

        //  Open the current Gedcom file.
        currentGedcomFileName = fileNames[i];
        FILE *currentFile = fopen(currentGedcomFileName, "r");
        if (!currentFile) {
            addErrorToLog(errorLog, systemError, currentGedcomFileName, 0, "could not open file.");
            continue;
        }

        //  The file is open. Create its database.
        RecordIndex *index = createRecordIndex();
        appendListElement(listOfIndexes, index);

        //  Import the contents of the current Gedcom file.
        if (!importFromFile(currentFile, index, errorLog)) {
            if (debugging) {
                printf("There was an error importing from file %s.\n", fileNames[i]);
                printf("There should be errors in the log that say what's wrong.\n");
            }
            continue;
        }
    }

    //  Done trying to import all files. If there were any errors, log them now and do not
    //  create a database. If there were no errors create the database and continue.

    if (lengthList(errorLog) > 0) {
        if (debugging) printf("There were errors: the error log should follow.\n");
        outputErrorLog(errorLog);
        return false;  //  False should mean 'cannot load the database.'
    }

    //  Iterate over all the indexes, and transfer their contents to the datbase.
    for (int fileIndex = 0; fileIndex < lengthList(listOfIndexes); fileIndex++) {
        if (debugging) printf("There were no errors: should now create the database.\n");
        setupDatabase(listOfIndexes);
    }

    return true;
}

Database *simpleImportFromFile(FILE* file, ErrorLog *errorLog)
{
	if (debugging) printf("Entered simpleImportFromFile\n");
    //  Create a new database to hold this file's records.
    Database *database = createDatabase();
    ASSERT(database);

    //  Read the records and add them to the database.
    String msg;
    int recordCount = 0;
    GNode *root = firstNodeTreeFromFile(file, &msg);
    while (root) {
        recordCount++;
	//if (debugging) printf("Just read record %d\n", recordCount);
        storeRecord(database, root);
        root = nextNodeTreeFromFile(file, &msg);
    }
    printf("Read %d records.\n", recordCount);

    return database;
}

//  importFromFile -- Import the records from a Gedcom file into an index. After reading the
//    records rekey the records.
//--------------------------------------------------------------------------------------------------
bool importFromFile(FILE* file, RecordIndex *index, ErrorLog* errorLog)
//  file -- File to read Gedcom records from.
//  index -- Record index for the file.
//  errorLog -- Error log.
{
    GNode *conv;
    String msg;

    //  Map used to rekey the records. It maps original keys to their new values.
    StringTable *rekeyMap = createStringTable();

    //  Read the records and add them to the record index.
    GNode *root = firstNodeTreeFromFile(file, &msg);
    while (root) {

        //  Normalize the records, ignoring header and trailer records.
        if (!(conv = normalizeNodeTree(root))) {
            freeGNodes(root);
            root = nextNodeTreeFromFile(file, &msg);
            continue;
        }

        //  Get the new key for the record.
        String newKey = strsave(updateKeyMap(root, rekeyMap));
        stdfree(root->key);
        root->key = strsave(newKey);

        //  Store the record in the file based database. The key of the root node has been
        //    rekeyed, but the keys in the value files have not been yet.
        //storeRecord(index, root);  //  TODO: Add code to really add the record to the index.

        //  Get the next record and loop.
        root = nextNodeTreeFromFile(file, &msg);
    }

    //  All records are now in the index. Rekey all the value keys in the records.
    if (debugging) {
        printf("Rekeying the records from file %s.\n", currentGedcomFileName);
        printf("Using the key map:\n"); showStringTable(keyMap);
    }
    //rekeyDatabase(database, rekeyMap);

    //  Validate the records in the index.
    if (debugging) printf("Validating the records from file %s.\n", currentGedcomFileName);
    //validateDatabase(database);

    //  Move the records in the current index to the database.
    if (lengthList(errorLog) == 0) {
        if (debugging) printf("Moving elements from the index to the database.\n");
        //addDatabaseToDatabase(database, theDatabase);
        //moveToDatabase(recordIndex);
    }

    //  TODO: Store all the records in the local recort index in the database.
    //  Remove the record index and rekey map.
    deleteHashTable(rekeyMap);
    return true;
}



String misnam = (String) "Missing NAME line in INDI record; record ignored.\n";
String noiref = (String) "FAM record has no INDI references; record ignored.\n";

// normalizeNodeTree -- Normalize node tree records to standard format.
//--------------------------------------------------------------------------------------------------
static GNode *normalizeNodeTree(GNode *root)
//  root -- Root of a gedcom node tree record.
{
    //  Don't worry about HEAD or TRLR records.
    if (eqstr("HEAD", root->tag) || eqstr("TRLR", root->tag)) return null;

    //  Normalize the node tree records to standard format.
    switch (recordType(root)) {
        case GRPerson: return normalizePerson(root);
        case GRFamily:  return normalizeFamily(root);
        case GREvent: return normalizeEvent(root);
        case GRSource: return normalizeSource(root);
        case GROther: return normalizeOther(root);
        default: FATAL();
    }
    return null;
}



//  updateKeyMap -- Update the key map by mapping an original key to a sequential key.
//--------------------------------------------------------------------------------------------------
static String updateKeyMap(GNode *root, StringTable *rekeyMap)
//  root -- Root node of a record.
{
    static char newKey[20];

    switch (recordType(root)) {
        case GRPerson:
            sprintf(newKey, "@I%d@", ++numPersons);
            break;
        case GRFamily:
            sprintf(newKey, "@F%d@", ++numFamilies);
            break;
        case GRSource:
            sprintf(newKey, "@S%d@", ++numSources);
            break;
        case GREvent:
            sprintf(newKey, "@E%d@", ++numEvents);
            break;
        case GROther:
            sprintf(newKey, "@X%d@", ++numOthers);
            break;
        case GRUnknown:
        case GRHeader:
        case GRTrailer:
            return null;
    }
    insertInStringTable(rekeyMap, root->key, newKey);
    return newKey;  //  MNOTE: new key is in static memory. Caller must save it.
}

//  rekeyTraverseFunction -- Function called on each record node during traversal. It looks
//    for values with key syntax (surrounded by @-signs) and remaps them.
//--------------------------------------------------------------------------------------------------
static StringTable *map;
static bool rekeyTraverseFunction(GNode* node)
{
    //printf("rekeyTraverseFunction called.\n");
    if (isKey(node->value)) {
        //  Lookup the key in the table.
        if (debugging) printf("Found a node with a key value of %s\n", node->value);
        String newKey = searchStringTable(map, node->value);
        if (!newKey) {
            //printf("The original key %s does not have a corresponding new key\n", node->value);
            //  TODO: Is it an error if there is a value that has key format but isn't defined?
            return true;
        }
        stdfree(node->value);
        node->value = strsave(newKey);
    }
    return true;
}

//  rekeyIndex -- All values that are have key values (strings surrounded by @-signs) are rekeyed
//    based on the rekey table.
//--------------------------------------------------------------------------------------------------
void rekeyIndex(RecordIndex *index, StringTable *rekeyMap)
//  index -- Record index for a file.
//  rekayMap -- rekey map for the file.
{
    int i, j;
    map = rekeyMap;
    RecordIndexEl *element = (RecordIndexEl*) firstInHashTable(index, &i, &j);
    while (element) {
        traverseNodes(element->root, rekeyTraverseFunction);
        rekeyTraverseFunction(element->root);
        element = (RecordIndexEl*) nextInHashTable(index, &i, &j);
    }
}

//  outputErrorLog
//--------------------------------------------------------------------------------------------------
static void outputErrorLog(ErrorLog* errorLog)
{
    sortList(errorLog, true);
    for (int index = 0; index < lengthList(errorLog); index++) {
        Error *error = getListElement(errorLog, index);
        printf("Error in file: %s at line %d: %s\n", error->fileName, error->lineNumber,
               error->message);
    }
}

static void setupDatabase(List *recordIndexes)
{
    theDatabase = createDatabase();
    
    for (int i = 0; i < lengthList(recordIndexes); i++) {
        RecordIndex *index = (RecordIndex*) getListElement(recordIndexes, i);
        addIndexToDatabase(index, theDatabase);
    }
}

//  addIndexToDatabase -- Add all the records in an index to a database.
//--------------------------------------------------------------------------------------------------
static void addIndexToDatabase(RecordIndex *index, Database *database)
//  index -- Record index for a file.
//  database -- Database to add the index's records to.
{
    int i, j;
    RecordIndexEl *element = (RecordIndexEl*) firstInHashTable(index, &i, &j);
    while (element) {
        storeRecord(database, element->root);
        element = (RecordIndexEl*) nextInHashTable(index, &i, &j);
    }
}
