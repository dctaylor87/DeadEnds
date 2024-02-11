//extern void change_node_tag (NODE node, STRING newtag);
extern GNode *create_temp_node (STRING xref, STRING tag, STRING val, NODE prnt);
extern void free_temp_node_tree (NODE node);
extern BOOLEAN is_temp_node (NODE node);
extern void set_temp_node (NODE node, BOOLEAN temp);
extern INT tree_strlen (INT levl, NODE node);
extern void unknown_node_to_dbase (NODE node); /* XXX */
extern int next_spouse (NODE *node, RECORD *spouse);
extern RECORD indi_to_prev_sib (RECORD irec);
extern RECORD indi_to_next_sib (RECORD irec);
extern STRING node_to_tag (NODE node, STRING tag, INT len);
extern void record_to_date_place (RECORD record, STRING tag,
				  STRING * date, STRING * plac, INT * count);
extern NODE record_to_first_event (RECORD record, CNSTRING tag);
extern NODE node_to_next_event (NODE node, CNSTRING tag);
extern void event_to_date_place (NODE node, STRING * date, STRING * plac);
extern STRING event_to_string (NODE node, RFMT rfmt);
extern void show_node (NODE node);
extern void show_node_rec (INT levl, NODE node);
extern INT length_nodes (NODE node);
extern NODE copy_node (NODE node);
extern NODE copy_node_subtree (NODE node);
extern NODE copy_nodes (NODE node, BOOLEAN kids, BOOLEAN sibs);
extern BOOLEAN traverse_nodes (NODE node, BOOLEAN (*func)(NODE, VPTR), VPTR param);
extern INT num_spouses_of_indi (NODE indi);
extern NODE find_node (GNode *prnt, String tag, String val, GNode **plast); /* gnode.c */
//extern void check_node_leaks (void);
extern void term_node_allocator (void);

#define nflag(node)	(node)->flags

#define ND_TEMP		0x1	/* node is a temp node, not part of a record */
