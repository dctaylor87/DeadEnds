//extern void change_node_tag (GNode *node, String newtag);
extern int tree_strlen (int levl, GNode *node);
extern void unknown_node_to_dbase (GNode *node); /* XXX */
extern int next_spouse (GNode **node, RecordIndexEl **spouse, Database *database);
extern RecordIndexEl *indi_to_prev_sib (RecordIndexEl *irec, Database *database);
extern RecordIndexEl *indi_to_next_sib (RecordIndexEl *irec, Database *database);
extern String node_to_tag (GNode *node, CString tag, int len);
extern void record_to_date_place (RecordIndexEl *record, String tag,
				  String * date, String * plac, int * count);
extern GNode *record_to_first_event (RecordIndexEl *record, CString tag);
extern GNode *node_to_next_event (GNode *node, CString tag);
extern void event_to_date_place (GNode *node, String * date, String * plac);
//extern String event_to_string (GNode *node, RFMT rfmt);
//extern void show_node (GNode *node);
//extern void show_node_rec (int levl, GNode *node);
extern GNode *copy_node_subtree (GNode *node);
extern bool traverse_nodes (GNode *node, bool (*func)(GNode *, Word), Word param);
//extern int num_spouses_of_indi (GNode *indi); /* gnode.h */
//extern GNode *find_node (GNode *prnt, String tag, String val, GNode **plast); /* gnode.c */
//extern void check_node_leaks (void);
extern void term_node_allocator (void);
