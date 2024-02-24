/* browse.h -- this header file is unique to the DE port of the LL interface */

/* browse.c */
extern void init_browse_lists(void);
extern void term_browse_lists(void);

/* brwslist.c */
extern void add_browse_list(String, Sequence *);
extern void remove_browse_list(String, Sequence *);
extern void new_name_browse_list(String, String);
extern void update_browse_list(String, Sequence *);
extern void remove_from_browse_lists(String);
extern void rename_from_browse_lists(String);
extern Sequence *stringToSequence (String name, Database *database);
extern Sequence *find_named_seq(String);


//extern void rename_indiseq(Sequence *, String);
