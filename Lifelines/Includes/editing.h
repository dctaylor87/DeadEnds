extern String editfile;

extern void write_fam_to_file_for_edit(GNode *fam, CString file, RFMT rfmt);
extern void write_nodes(INT, FILE*, XLAT, GNode *, BOOLEAN, BOOLEAN, BOOLEAN);
extern void write_node_to_editfile(GNode *);
