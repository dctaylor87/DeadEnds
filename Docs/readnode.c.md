# readnode.c

|Component|Description|
|:---|:---|
|int fileLine = 0|Current line in file being read.|
|int fileToLine(FILE *file, int *plevel, String *pkey, String *ptag, String *pvalue, String *pmessage)|Read the next Gedcom line from a file.|
|static bool stringToLine(String *ps, int *plevel, String *pkey, String *ptag, String *pvalue, String *pmessage)|Get the next Gedcom line from a string holding one or more lines. It reads to the next newline, if any, and processes that part of the  string. If there are remaining characters the address of the next character is returned in ps.|
|static int bufferToLine (String p, int* plevel, String* pkey, String* ptag, String* pvalue, String* pmessage)|Process a Gedcom line, extracting the level, the key, if any, the tag, and the value, if any. Called by both fileToLine and stringToLine.|
|GNode* firstNodeTreeFromFile (FILE* fp, String* pmsg)|Convert the first Gedcom record in a file to a GNode tree.|
|GNode* nextNodeTreeFromFile(FILE *fp, String *pmsg)|Convert the next Gedcom record in a file to a GNode tree.|
|GNode* stringToNodeTree(String str)|Convert a string holding a  Gedcom record to a GNode tree. _Was used by LifeLines when reading records from its database. Not yet needed by DeadEnds_.|