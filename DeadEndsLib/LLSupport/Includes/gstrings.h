extern String *get_child_strings (GNode *fam, bool rfmt,
				  int *pnum, String **pkeys, Database *database);
extern String indi_to_list_string (GNode *indi, GNode *fam, int len,
				   bool rfmt, bool appkey);
extern String sour_to_list_string (GNode *sour, int len, String delim);
extern String even_to_list_string (GNode *even, int len, String delim);
extern String fam_to_list_string (GNode *fam, int len, String delim,
				  Database *database);
extern String other_to_list_string(GNode *node, int len, String delim);
extern String generic_to_list_string (GNode *node, CString key, int len,
				      String delim, bool rfmt, bool appkey,
				      Database *database);
extern void set_displaykeys (bool keyflag);
