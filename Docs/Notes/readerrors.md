# Errors and the Reading Gedcom Files

## Introduction

The code that imports databases uses functions in import.c and readnode.c

These notes describe what the stack of code does when importing a database from a Gedcom file.

Some of the problems can be understood in the context of who knows what and when? Some of the issues are:

1. When reading Gedcom files, several types of errors can occur. Some are final in the sense that the program cannot continue (failure to open or read files).
1. Some involve the syntax of lines in the Gedcom file. Though errors here must prevent the file from being imported (that is, no database should be created for the file), the program should not quit, so that further errors can be found.
1. Other errors can occur at higher levels. For example, certain line types require formatted values (e.g., NAME and SEX lines in person records). Lineage linking requires that roles in person and family records refer, by key value, to specific, existing family and person records. Source links in records must refer, by key value, to existing source records. None of these errors are in the syntax of Gedcom. (*Gedcom standards refer to keys as cross-reference identifiers. This is too wordy for DeadEnds, where they## are called keys, or record keys when context requires.*)
1. Errors must be reported to the user in a way that the errors can be easily understood, found in the Gedcom file, and fixed. For this to happen the program must:

Know the file name and the line number in the files where the errors occur.

Create error objects that are added to a log for eventual display.


Errors of different types may be added to the log during different phases, so the logs must be sortable to present the errors in a logical fashion.


### Read Stack
The read stack is a layering of software functions that transform Gedcom records from characters in a Unix file to GNode trees in a DeadEnds database.
    
#### List *importFromFiles(String[] fileNames, ErrorLog *errorLog)
*importFromFiles* is the function at the top of the stack. It is passed an array of file names, and an error log. It then creates a DeadEnds database for each file. If errors are encountered they are added to the error log. Depending on the number and type of errors, databases from one or more files may not be created. This function calls *importFromFile* with each file name, so this function is not needed in applications that read a single file; they call *importFromFile* directly.

#### Database *importFromFile(String fileName, ErrorLog *errorLog)
*importFromFile* is passed the name of a Gedcom file and an error log. It reads the file, breaking it into Gedcom records which are added to database. If errors are encountered they are added to the error log. Depending on the types of errors, the database may not be created; if this happens the function returns null.

The function calls *fopen* to open the file for reading. The file handle is passed to the lower layers. The function also calls *createDatabase* to create the database that is returned.

To get records from the file, *importFromFile* calls *firstNodeTreeFromFile* and *nextNodeTreeFromFile*, the former being called only once to set up state variables. After each call to these functions, *importFromFile* calls *storeRecode* to put the just read record into the database.

#### GNode *firstNodeTreeFromFile(FILE *fp, String *errorMsg)
*firstNodeTreeFromFile* sets up the variables that maintain state between successive calls to *nextNodeTreeFromFile*. After setting the state, *firstNodeTreeFromFile* calls *fileToLine* to get the first Gedcom line from the file into state variables, and then calls *nextNodeTreeFromFile* to get the the first record, which it returns to its caller.

 *The second parameter to firstNodeTreeFromFile and nextNodeTreeFromFile should be an ErrorLog, not a String error message*. This is because many errors can occur during the reading of a Gedcom record. This change is underway now.

#### GNode *nextNodeTreeFromFile(FILE *fp, String *errorMsg)
The main task of *nextNodeTreeFromFile* is to read all the lines from the Gedcom file that make up a single Gedcom record, represented as a GNode tree, and return it to its caller. To get each line it calls the *fileToLine* function.
    
The first parameter is the file to read from, and the second parameter is a String error message that can be returned. *However, as for firstNodeTreeFromFile, this parameter must be changed to an ErrorLog*.

The complexity found in *nextNodeTreeFromFile* is caused by its need to keep track of the level numbers of each line in the Gedcom record, so the GNode tree it builds and returns is structured properly.

When *nextNodeTreeFromFile* is called, the first line of the new record will have been read previously and put in the state variables (by *firstNodeTreeFromFile* for the first record; by *nextNodeTreeFromFile* for all others.)

*nextNodeTreeFromFile* calls *fileToLine* iteratively to read the lines making up a Gedcom record. When the record is not the last one in the file, *nextNodeTreeFromFile* calls *fileToLine* an extra time, as it does not know when the next 0 level line will be found. To prevent rereading the file, the contents of this 0 level line make up the state saved between calls to *nextNodeTreeFromFile*.

*nextNodeTreeFromFile* also calls *createGNode* to build the GNode tree that is returned. If there are errors, the function may also call *deleteGNode*

#### static int fileToLine(FILE *fp, int *level, String *key, String *tag, String *value, String *errorMsg)

*fileToLine* gets the next line from the Gedcom file, and places the four fields of the line (key, level, tag and value) in output parameters. If an error occurs reading the line an error message is also returned. The functional return value is an integer code, for NORMAL, ERROR, and ATEOF. (*This is not currently true.*)

*fileToLine* uses *fgets* to read the next line from the file. If *fgets* returns 0 the file is at end of file and *fileToLine* returns the EOF code. If *fgets* does get characters from the file it puts them in a buffer and then calls *bufferToLine* to scan the line and extract the four fields.

*fileToLine* calls *bufferToLine* to do the lexical work extracting the fields.
    
#### static int bufferToLine (String p, int* level, String* key, String* tag, String* value, String* errorMsg)

*bufferToLine* processes a Gedcom line, extracting the key (if any), level, tag, and value (if any) fields. The line may have a newline at the end. This function is called by both *fileToLine* and *stringToLine* to the lexical work required to extract the field.
