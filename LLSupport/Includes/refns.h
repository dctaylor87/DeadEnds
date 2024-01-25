extern INT resolveRefnLinks (GNode *node, Database *database);
extern bool addRefn (CString refn, CString key, Database *database);
extern bool removeRefn (CString refn, CString key, Database *database);
extern CString getRefn (CString refn, Database *database);
extern bool indexByRefn (GNode *node, Database *database); /* needs a better name! */
extern void annotateWithSupplemental (GNode *node, RFMT rfmt, Database *database); /* should it copy the tree? */
typedef bool (*TRAV_REFNS_FUNC)(CString key, CString refn, Word param, Database *database);
extern void traverseRefns (TRAV_REFNS_FUNC func, Word param, Database *database);
