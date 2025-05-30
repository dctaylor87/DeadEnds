GNode *rptui_ask_for_fam(CString s1, CString s2, Database *);
Sequence *rptui_ask_for_indi_list(CString ttl, bool reask, Database *database);
GNode *rptui_ask_for_indi (CString ttl, ASK1Q ask1, Database *database);
CString rptui_ask_for_indi_key(CString ttl, ASK1Q ask1, Database *database);
bool rptui_ask_for_int(CString, int *);
FILE * rptui_ask_for_output_file(CString mode, CString ttl, String *pfname,
				 CString path, CString ext);
bool rptui_ask_for_program(CString mode, CString ttl, String *pfname,
			   CString path, CString ext, bool picklist);
int rptui_chooseFromArray(CString ttl, int no, String *pstrngs);
int rptui_elapsed(void);
void rptui_init(void);
int rptui_prompt_stdout(CString prompt);
void rptui_view_array(CString ttl, int no, String *pstrngs);
