#define NAMESEP		'/'	/* separates surname from the rest of the name */

extern void prefix_file_for_edit (FILE *fp);
extern void write_fam_to_file_for_edit(GNode *fam, CString file, bool rfmt, Database *database);
extern void write_indi_to_file_for_edit(GNode *indi, CString file, bool rfmt, Database *database);
extern void write_nodes(int, FILE*, XLAT, GNode *, bool, bool, bool);
extern void write_node_to_editfile(GNode *);

extern bool equal_tree(GNode *, GNode *);
extern bool equal_node(GNode *, GNode *);
extern bool equal_nodes(GNode *, GNode *, bool, bool);

extern bool iso_list(GNode *, GNode *);
extern bool iso_nodes(GNode *, GNode *, bool, bool );
