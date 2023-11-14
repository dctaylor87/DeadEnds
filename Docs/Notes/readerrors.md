# Reading Gedcom Files and Handling Errors

## Introduction

These notes describe the stack of code that imports Gedcom records from Unix files and converts them into GNode trees in DeadEnds databases, and the errors that can occur during importing, and how those errors are handled. *This is an area in flux*.

Errors that can occur during importing are:

1. System errors such as failing to open a file, or read a file, or allocate memory.1. Errors that involve the syntax of the lines in a Gedcom file. Though errors here must prevent the file from being imported, the program should not quit, so that later errors can be found.
1. Errors can occur at higher levels. For example, certain line types require formatted values (e.g., NAME and SEX lines in persons).
1. Erros can occur in the inter-record links. Lineage linking requires that roles in person and family records (FAMS, FAMC, CHIL, WIFE, HUSB) refer, by key value, to other records. Source links in records must refer to existing source records. (*Gedcom standards refer to keys as cross-reference identifiers*.)

Errors must be reported to the user in a way that they can be  understood, found in the Gedcom files, and fixed. For this to happen the program must know the file name and the line numbers where the errors occur. DeadEnds uses the *ErrorLog* type to hold the errors. Errors of different types may be added to the log during different phases, so logs are sortable to organize the errors in a logical fashion.


### Read Stack
The read stack is the layering of software functions that transform Gedcom records from characters in a Unix file to GNode trees in a DeadEnds database. This section gives an introduction and description of the software functions that compose the stack.
    
#### List *importFromFiles(String[] fileNames, ErrorLog *errorLog)
*importFromFiles* is the function at the top of the stack. It is passed an array of file names, and an error log. It calls *importFromFile* on each file. It gathers the databases returned by that function into a list of databases, that it then returns. If errors are encountered they are added to the error log. Depending on the number and type of errors, databases from one or more files may not be created. If an application only needs to read a single file into a single database, it should call *importFromFile* directly.

#### Database *importFromFile(String fileName, ErrorLog *errorLog)
*importFromFile* is passed the name of a Gedcom file and an error log. It reads the file, breaking it into Gedcom records which are added a database that it creates. If errors are encountered they are added to the error log. If there are errors the database is not be created and this function returns null.

*importFromFile* calls *fopen* to open the file for reading. The file handle is passed to lower layers. The function also calls *createDatabase* to create the database that is returned.

To read the records from the file, *importFromFile* calls *firstNodeTreeFromFile* and *nextNodeTreeFromFile*, the former being called only once to set up state variables. After each call to these functions, *importFromFile* calls *storeRecorde* to put the record in the database.

#### GNode *firstNodeTreeFromFile(FILE *fp, String *errorMsg)
*firstNodeTreeFromFile* sets up the variables that maintain state between successive calls to *nextNodeTreeFromFile*. After setting the state, *firstNodeTreeFromFile* calls *fileToLine* to get the first Gedcom line from the file into state variables, and then calls *nextNodeTreeFromFile* to get the the first record, which it returns to its caller.

 *The second parameter to firstNodeTreeFromFile and nextNodeTreeFromFile should be an ErrorLog, not a String error message*. This is because many errors can occur during the reading of a Gedcom record. This change is underway now.

#### GNode *nextNodeTreeFromFile(FILE *fp, String *errorMsg)
The main task of *nextNodeTreeFromFile* is to read all the lines from the Gedcom file that make up the next Gedcom record in the, represent it as a GNode tree, and return it to its caller. To get each line it calls the *fileToLine* function.
    
The first parameter is the file to read from, and the second parameter is a String error message that can be returned. *However, as for firstNodeTreeFromFile, this parameter must be changed to an ErrorLog*.

The complexity in *nextNodeTreeFromFile* is caused by its need to track of the levels of each line in the Gedcom record, so it can build the record's GNode tree with the correct structure.

When *nextNodeTreeFromFile* is called, the first line of the new record will have been previosly read and placed in the state variables (by *firstNodeTreeFromFile* for the first record; by the preceding call to *nextNodeTreeFromFile* for all others.)

*nextNodeTreeFromFile* calls *fileToLine* iteratively to read the lines making up a Gedcom record. When the record is not the last one in the file, *nextNodeTreeFromFile* will call *fileToLine* an extra time, as it does not know when the next 0 level line will be found. To prevent rereading the file, the contents of this 0 level line is the state data saved between calls to *nextNodeTreeFromFile*.

*nextNodeTreeFromFile* also calls *createGNode* to build the GNode tree that is returned. If there are errors, the function may also call *deleteGNode*

#### static int fileToLine(FILE *fp, int *level, String *key, String *tag, String *value, String *errorMsg)

*fileToLine* gets the next line from the Gedcom file, and places the four fields of the line (key, level, tag and value) in output parameters. If an error occurs reading the line an error message is also returned. The functional return value is an integer code, for NORMAL, ERROR, and ATEOF. (*This is not currently true.*)

*fileToLine* uses *fgets* to read the next line from the file. If *fgets* returns 0 the file is at end of file and *fileToLine* returns the EOF code. If *fgets* does get characters from the file it puts them in a buffer and then calls *bufferToLine* to scan the line and extract the four fields.

*fileToLine* calls *bufferToLine* to do the lexical work extracting the fields.
    
#### static int bufferToLine (String p, int* level, String* key, String* tag, String* value, String* errorMsg)

*bufferToLine* processes a Gedcom line, extracting the key (if any), level, tag, and value (if any) fields. The line may have a newline at the end. This function is called by both *fileToLine* and *stringToLine* to the lexical work required to extract the field.
