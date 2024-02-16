//extern void change_node_tag (GNode *node, String newtag);
extern GNode *create_temp_node (String xref, String tag, String val, GNode *prnt);
extern void free_temp_node_tree (GNode *node);
extern bool is_temp_node (GNode *node);
extern void set_temp_node (GNode *node, bool temp);
extern INT tree_strlen (INT levl, GNode *node);
extern void unknown_node_to_dbase (GNode *node); /* XXX */
extern int next_spouse (GNode **node, RECORD *spouse, Database *database);
extern RECORD indi_to_prev_sib (RECORD irec, Database *database);
extern RECORD indi_to_next_sib (RECORD irec, Database *database);
extern String node_to_tag (GNode *node, String tag, INT len);
extern void record_to_date_place (RECORD record, String tag,
				  String * date, String * plac, INT * count);
extern GNode *record_to_first_event (RECORD record, CString tag);
extern GNode *node_to_next_event (GNode *node, CString tag);
extern void event_to_date_place (GNode *node, String * date, String * plac);
//extern String event_to_string (GNode *node, RFMT rfmt);
extern void show_node (GNode *node);
extern void show_node_rec (INT levl, GNode *node);
extern GNode *copy_node_subtree (GNode *node);
extern bool traverse_nodes (GNode *node, bool (*func)(GNode *, Word), Word param);
//extern INT num_spouses_of_indi (GNode *indi); /* gnode.h */
extern GNode *find_node (GNode *prnt, String tag, String val, GNode **plast); /* gnode.c */
//extern void check_node_leaks (void);
extern void term_node_allocator (void);

#define nflag(node)	(node)->flags

#define ND_TEMP		0x1	/* node is a temp node, not part of a record */
