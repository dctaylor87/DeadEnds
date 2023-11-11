//
//  DeadEnds
//
//  readnode.c -- Functions that read Gedcom nodes and node trees from files and strings.
//
//  Created by Thomas Wetmore on 17 December 2022.
//  Last changed on 11 November 2023.
//

#include "readnode.h"
#include "stringtable.h"

//  Error messages used in this file.
//--------------------------------------------------------------------------------------------------
static String fileof = (String) "The file is positioned at EOF.";
static String reremp = (String) "Line %d: This line is empty; EOF?";
static String rerlng = (String) "Line %d: This line is too long.";
static String rernlv = (String) "Line %d: This line has no level number.";
static String rerinc = (String) "Line %d: This line is incomplete.";
static String rerbln = (String) "Line %d: This line has a bad link.";
//static String rernwt = (String) "Line %d: This line needs white space before tag.";
static String rerilv = (String) "Line %d: This line has an illegal level.";
static String rerwlv = (String) "The record begins at wrong level.";

//  Local static functions.
//--------------------------------------------------------------------------------------------------
static int bufferToLine (String, int*, String*, String*, String*, String*);

//  Local static variables.
//--------------------------------------------------------------------------------------------------
int fileLine = 0;  // Current line in the file being read.

//  fileToLine -- Reads the next Gedcom line from a file. Empty lines are counted and ignored.
//    The line is passed to bufferToLine for field extraction. An error message is returned if
//    a problems is found. Returns a code of be OKAY, DONE, or ERROR. The function uses fgets()
//    to read lines from the file.
//--------------------------------------------------------------------------------------------------
static int fileToLine(FILE *file, int *plevel, String *pkey, String *ptag, String *pvalue,
			   String *pmessage)
//  file -- File to read the line from.
//  plevel -- (out) Pointer to level of the returned line.
//  pxref -- (out) Pointer to cross reference string of the returned line.
//  ptag -- (out) Pointer to the tag string of the returned line.
//  pval -- (out) Pointer to the value string of the returned line.
//  pmsg -- (out) Pointer to an error message when things go wrong.
{
	static char buffer[MAXLINELEN];  //  Buffer to store the line.
	char *p = buffer;  //  Buffer cursor.
	*pmessage = null;
	while (true) {
		//  Read a line from the file; if fgets returns 0 assume reading is over.
		if (!(p = fgets(buffer, MAXLINELEN, file))) return DONE;
		fileLine++;  // Increment the file line number.
		if (!allwhite(p)) break; // If the line is all white continue to the next line.
	}

	// Read a line and convert it to field values. The values point to locations in the buffer.
	return bufferToLine(p, plevel, pkey, ptag, pvalue, pmessage);
}

//  stringToLine -- Get the next Gedcom line as fields from a string holding one or more Gedcom
//    lines. This function reads to the next newline, if any, and processes that part of the
//    string. If there are remaining characters the address of the next character is returned
//    in ps
//--------------------------------------------------------------------------------------------------
static bool stringToLine(String *ps, int *plevel, String *pkey, String *ptag, String *pvalue,
						 String *pmessage)
//  ps -- (in/out) Pointer to string.
//  plevel -- (out) Pointer to level.
//  pxref -- (out) Pointer to cross reference string of this line.
//  ptag -- (out) Pointer to tag of this line.
//  pval -- (out) Pointer to value of this line.
//  pmsg -- (out) Pointer to error message if anything goes wrong.
{
	String s0, s;
	*pmessage = null;
	s0 = s = *ps;
	if (!s || *s == 0) return false;
	while (*s && *s != '\n') s++;
	if (*s == 0)
		*ps = s;
	else {
		*s = 0;
		*ps = s + 1;
	}
	return bufferToLine(s0, plevel, pkey, ptag, pvalue, pmessage);
}

//  bufferToLine -- Process a Gedcom line, extracting the level, the key, if any, the tag, and
//    the value, if any. The line may have a newline at the end. This function is called by both
//    fileToLine and stringToLine.
//--------------------------------------------------------------------------------------------------
static int bufferToLine (String p, int* plevel, String* pkey, String* ptag, String* pvalue,
						   String* pmessage)
//  p -- Gedcom line before processing. Within the function p is used as a cursor.
//  plevel -- (out) Pointer to line's Gedcom level.
//  pkey -- (out) Pointer to line's key if any.
//  ptag -- (out) Pointer to line's tag.
//  pvalue -- (out) Pointer to line's value, if any.
//  pmessage -- (out) Pointer to error message, if any, in a static buffer.
{
	// Static buffer for returning error messages.
	static char scratch[MAXLINELEN+40];

	// Be sure a string was passed in.
	if (!p || *p == 0) {
		sprintf(scratch, reremp, fileLine);
		*pmessage = scratch;
		return ERROR;
	}
	// Initialize the output parameters.
	*pmessage = *pkey = *pvalue = 0;
	// Strip trailing white space from the String. TODO: THIS SEEMS WASTEFUL.
	striptrail(p);

	// Check that the input string isn't too long.
	if (strlen(p) > MAXLINELEN) {
		sprintf(scratch, rerlng, fileLine);
		*pmessage = scratch;
		return ERROR;
	}

	// Get the level number. Pass any whitespace that precedes the level.
	while (iswhite(*p)) p++;

	// The first non-white character must be a digit for the Gedcom's line level.
	if (chartype(*p) != DIGIT) {
		sprintf(scratch, rernlv, fileLine);
		*pmessage = scratch;
		return ERROR;
	}

	// Use ascii arithmetic to convert the digit characters to integers.
	int level = *p++ - '0';
	
	// Though extremely unlikely, handle levels with multiple digits.
	while (chartype(*p) == DIGIT) level = level*10 + *p++ - '0';
	
	// Set the output level parameter to its value.
	*plevel = level;

	// Pass any white space that precedes the key or tag.
	while (iswhite(*p)) p++;

	// If at the end of the string it is an error.
	if (*p == 0) {
		sprintf(scratch, rerinc, fileLine);
		*pmessage = scratch;
		return ERROR;
	}

	// If @ is the first character, this line has a key.
	if (*p != '@') goto gettag;  // No @-sign. Skip to the tag.

	// Get the key. Include the @-signs, considering them as syntactic sugar.
	*pkey = p++;  // Don't skip the '@'. MNOTE: The returned key points into the original string.
	if (*p == '@') {  // A Gedcom key of @@ is illegal.
		sprintf(scratch, rerbln, fileLine);
		*pmessage = scratch;
		return ERROR;
	}
	while (*p != '@' && *p != 0) p++;  // Read until the second @-sign.
	//  If at the end of the string it is an error.
	if (*p == 0) {
		sprintf(scratch, rerinc, fileLine);
		*pmessage = scratch;
		return ERROR;
	}
	//  p now points to the second @-sign. Put a space into the next character (which will be
	//    between the 2nd @ and the first character of the tag).
	if (*++p != ' ') {
		sprintf(scratch, "%d: There must be a space between the key and the tag.\n", fileLine);
		printf("There must be a space between the key and the tag.\n");
		*pmessage = scratch;
	}
	*p++ = 0;
	// There must be white space between the key and the tag.
//    if (!iswhite(*p++)) {
//        sprintf(scratch, rernwt, fileLine);
//        *pmessage = scratch;
//        return ERROR;
//    }

	// Get the tag field.
gettag:
	while (iswhite(*p)) p++;  // Allow additional white space.
	if ((int) *p == 0) {
		sprintf(scratch, rerinc, fileLine);
		*pmessage = scratch;
		return ERROR;
	}
	*ptag = p++;  // MNOTE: The returned tag points into the original string.
	while (!iswhite(*p) && *p != 0) p++;
	if (*p == 0) return OKAY;
	*p++ = 0;

	// Get the value field.
	while (iswhite(*p)) p++;
	*pvalue = p;  // MNOTE: The returned value points into the original string.
	return OKAY;
}

//  Static variables that maintain state between firstNodeTreeFromFile and nextNodeTreeFromFile.
//--------------------------------------------------------------------------------------------------
static int level;
static String key;
static String tag;
static String value;
static bool ateof = false;

// firstNodeTreeFromFile -- Convert first Gedcom record in a file to a gedcom node tree.
//--------------------------------------------------------------------------------------------------
GNode* firstNodeTreeFromFile (FILE* fp, String* pmsg)
//  fp -- File that holds the gedcom records.
//  pmsg -- (out) Possible error message.
//  peof -- (out) Set to true if the file is at end of file.
{
	//ateof = false;
	fileLine = 0;
	*pmsg = null;
	int rc = fileToLine(fp, &level, &key, &tag, &value, pmsg);
	if (rc == DONE) {
		ateof = true;
		*pmsg = fileof;
		return null;
	} else if (rc == ERROR)
		return null;
	return nextNodeTreeFromFile(fp, pmsg);
}

//  nextNodeTreeFromFile -- Convert the next Gedcom record in a file to a Node tree.
//--------------------------------------------------------------------------------------------------
GNode* nextNodeTreeFromFile(FILE *fp, String *pmsg)
//  fp -- File that holds the Gedcom records.
//  pmsg -- (out) Possible error message.
//  peof -- (out) Set to true if the file is at end of file.
{
	int bcode, rc;
	GNode *root, *node, *curnode;
	static char scratch[100];
	*pmsg = null;
	//*peof = false;
	// If file is at end return null.
	if (ateof) {
		//ateof = *peof = true;
		*pmsg = fileof;
		return null;
	}

	//  The first line in the record has been read and must have level 0.
	int curlev = level;
	if (curlev != 0)  {
		*pmsg = rerwlv;
		return null;
	}

	//  Create the root of a node tree.
	root = curnode = createGNode(key, tag, value, null);
	bcode = OKAY;

	//  Read the lines of the current record and build its tree.
	rc = fileToLine(fp, &level, &key, &tag, &value, pmsg);
	while (rc == OKAY) {

		//  If the level is zero the the record has been read and built.
		if (level == 0) {
			bcode = DONE;
			break;
		}

		//  If the level of this line is the same as the last, add a sibling node.
		if (level == curlev) {
			node = createGNode(key, tag, value, curnode->parent);
			curnode->sibling = node;
			curnode = node;

			//  If the level of this line is one deeper than the last, add a child node.
		} else if (level == curlev + 1) {
			node = createGNode(key, tag, value, curnode);
			curnode->child = node;
			curnode = node;
			curlev = level;

			//  If the level of this line is less than the last then move up the parent chain until
			//  to the right level.
		} else if (level < curlev) {

			// Check for an illegal level.
			if (level < 0) {
				sprintf(scratch, rerilv, fileLine);
				*pmsg = scratch;
				bcode = ERROR;
				break;
			}

			//  Move up the parent list until reaching the node with the same level.
			while (level < curlev) {
				curnode = curnode->parent;
				curlev--;
			}

			//  Add the new node as a sibling.
			node = createGNode(key, tag, value, curnode->parent);
			curnode->sibling = node;
			curnode = node;

			//  Anything else is an error.
		} else {
			sprintf(scratch, rerilv, fileLine);
			*pmsg = scratch;
			bcode = ERROR;
			break;
		}
		//  The line was converted to a node and inserted. Read the next line and continue.
		rc = fileToLine(fp, &level, &key, &tag, &value, pmsg);
	}

	//  At the end of the loop. If the code was successful return the tree root.
	if (bcode == DONE) return root;
	if (bcode == ERROR || rc == ERROR) {
		// If there were errors free all nodes rooted at root and return null.
		freeGNodes(root);
		return null;
	}
	ateof = true ;
	return root;
}

//  stringToNodeTree -- Convert a string with a single Gedcom record into a node tree. Was used
//    by LifeLines when reading records from its database to convert them to node tree format.
//    So far not needed by DeadEnds because the records are never in a string format.
//--------------------------------------------------------------------------------------------------
GNode* stringToNodeTree(String str)
{
	int lev;
	int lev0;
	String xref;
	String tag;
	String val;

	int curlev;
	GNode *root, *node, *curnode;
	String msg;
	fileLine = 0;
	if (!stringToLine(&str, &lev, &xref, &tag, &val, &msg)) return null;
	lev0 = curlev = lev;
	root = curnode = createGNode(xref, tag, val, null);
	while (stringToLine(&str, &lev, &xref, &tag, &val, &msg)) {
		if (lev == curlev) {
			node = createGNode(xref, tag, val, curnode->parent);
			curnode->sibling = node;
			curnode = node;
		} else if (lev == curlev + 1) {
			node = createGNode(xref, tag, val, curnode);
			curnode->child = node;
			curnode = node;
			curlev = lev;
		} else if (lev < curlev) {
			if (lev < lev0) {
				printf("Error: line %d: illegal level", fileLine);
				return null;
			}
			while (lev < curlev) {
				curnode = curnode->parent;
				curlev--;
			}
			node = createGNode(xref, tag, val, curnode->parent);
			curnode->sibling = node;
			curnode = node;
		} else {
			printf("Error: line %d: illegal level", fileLine);
			return null;
		}
	}
	if (!msg) return root;
	freeGNodes(root);
	return null;
}
