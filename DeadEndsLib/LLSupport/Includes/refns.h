extern int resolveRefnLinks (GNode *node, Database *database);
extern bool addRefn (CString refn, CString key, Database *database);
extern bool removeRefn (CString refn, CString key, Database *database);
extern CString getRefn (CString refn, Database *database);
extern bool indexByRefn (GNode *node, Database *database); /* needs a better name! */
extern void annotateWithSupplemental (GNode *node, bool rfmt, Database *database); /* should it copy the tree? */
typedef bool (*TRAV_REFNS_FUNC)(CString key, CString refn, void* param, Database *database);
extern void traverseRefns (TRAV_REFNS_FUNC func, void* param, Database *database);

/* XXX these might be renamed, made static, or ... XXX */
extern int record_letter (CString tag);
extern GNode *refn_to_record (String ukey, int letr, Database *database);
