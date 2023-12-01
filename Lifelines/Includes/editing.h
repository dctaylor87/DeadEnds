#define NAMESEP		'/'	/* separates surname from the rest of the name */

extern String editfile;

extern void write_fam_to_file_for_edit(GNode *fam, CString file, RFMT rfmt);
extern void write_nodes(INT, FILE*, XLAT, GNode *, BOOLEAN, BOOLEAN, BOOLEAN);
extern void write_node_to_editfile(GNode *);

extern BOOLEAN equal_tree(GNode *, GNode *);
extern BOOLEAN equal_node(GNode *, GNode *);
extern BOOLEAN equal_nodes(GNode *, GNode *, BOOLEAN, BOOLEAN);

extern BOOLEAN iso_list(GNode *, GNode *);
extern BOOLEAN iso_nodes(GNode *, GNode *, BOOLEAN, BOOLEAN);
