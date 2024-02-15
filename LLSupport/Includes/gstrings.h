extern String *get_child_strings (GNode *fam, bool rfmt,
				  INT *pnum, String **pkeys);
extern String indi_to_list_string (GNode *indi, GNode *fam, INT len,
				   bool rfmt, bool appkey);
extern String sour_to_list_string (GNode *sour, INT len, String delim);
extern String even_to_list_string (GNode *even, INT len,
				   HINT_PARAM_UNUSED String delim);
extern String fam_to_list_string (GNode *fam, INT len, String delim);
extern String other_to_list_string(GNode *node, INT len,
				   HINT_PARAM_UNUSED String delim);
extern String generic_to_list_string (GNode *node, String key, INT len,
				      String delim, bool rfmt, bool appkey);
extern void set_displaykeys (bool keyflag);
