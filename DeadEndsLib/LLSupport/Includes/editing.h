#define NAMESEP		'/'	/* separates surname from the rest of the name */

extern void prefix_file_for_edit (FILE *fp);
extern void prefix_file_for_gedcom (FILE *fp);
extern void prefix_file_for_report (FILE *fp);
extern void write_indi_to_file (GNode *indi, CString file);
extern void write_fam_to_file_for_edit(GNode *fam, CString file, bool rfmt, Database *database);
extern void write_indi_to_file_for_edit(GNode *indi, CString file, bool rfmt, Database *database);
extern void write_nodes(int, FILE*, XLAT, GNode *, bool, bool, bool);
extern void write_node_to_editfile(GNode *);
