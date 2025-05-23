// DeadEnds Project
//
// gnode.h defines theGNode datatype. GNodes represent lines in a Gedcom file. GNodes are heap
// objects.
//
// Created by Thomas Wetmore on 4 November 2022.
// Last changed on 30 November 2024.

#ifndef gnode_h
#define gnode_h

typedef struct HashTable HashTable;  // Forward reference.
#define RecordIndex HashTable
typedef struct Database Database;
typedef enum SexType SexType;

#include "standard.h"
#include "gedcom.h"
#include "hashtable.h"
#include "database.h"
#include "recordindex.h"

// GNode is the structure that holds a Gedcom line in its tree node form. Root nodes have keys.
typedef struct GNode GNode;
struct GNode {
	String key;     // Record key; only root nodes use this field.
	String tag;     // Line tag; all nodes use this field.
	String value;   // Line value; values are optional.
	GNode *parent;  // Parent node; all nodes except roots use this field.
	GNode *child;   // First child none of this node, if any.
	GNode *sibling; // Next sibling node of this node, if any.
	uint32_t refcount; // Reference count
	uint8_t flags;  // flags, such as ND_TEMP
	uint8_t spare[3]; // reserved for future expansion
};

#define nchild(node)	((node)->child)
#define nsibling(node)	((node)->sibling)
#define nparent(node)	((node)->parent)
#define nval(node)	((node)->value)
#define ntag(node)	((node)->tag)
#define nxref(node)	((node)->key)
#define nflag(node)	(node)->flags

#define ND_TEMP		0x1	/* node is a temp node, not part of a record */

#define nrefcnt_inc(node)	((node)->refcount++)
#define nrefcnt_dec(node)	((node)->refcount--)
#define get_nrefcnt(node)	((node)->refcount)

// Application programming interface to this type.
GNode* createGNode(CString key, String tag, String value, GNode* parent);
void freeGNode(GNode*);
void freeGNodes(GNode*);
int gnodeLevel(GNode* node);

String gnodeToString(GNode*, int level);
String gnodesToString(GNode*);
int treeStringLength(int, GNode*);
GNode* personToFamilyAsChild(GNode *person, RecordIndex*);

String personToEvent(GNode*, String, String, int, bool);
String familyToEvent(GNode*, String, String, int, bool);
String eventToString(GNode*, bool);
String eventToDate(GNode*, bool);
String eventToPlace(GNode*, bool);
void showGNodeTree(GNode*);
void showGNodes(int, GNode*);
void showGNode(int level, GNode*);
int gNodesLength(GNode*);
String shortenDate(String);
String shortenPlace(String);
//static bool allDigits(String)
GNode* copyNode(GNode*);
GNode* copyNodes(GNode*, bool, bool);
void traverseNodes (GNode* node, int level, bool (*func)(GNode*, int));
int num_spouses_of_indi(GNode*);
GNode* findNode(GNode*, String, String, GNode**);

int countNodes(GNode* node);
int countNodesBefore(GNode*);

bool isKey(CString);
GNode* findTag(GNode*, CString);
SexType valueToSex(GNode*);
String full_value(GNode*);
String recordKey(GNode* node);

int numNodeAllocs(void);
int numNodeFrees(void);

// import.c
extern GNode *normalizeNodeTree (GNode*);

// temp-nodes.c
extern GNode *createTempGNode (String xref, String tag, String val, GNode *prnt);
extern void freeTempGNodeTree (GNode *node);
extern bool isTempGNode (GNode *node);
extern void setTempGNode (GNode *node, bool temp);

#endif // node_h
