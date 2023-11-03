extern int deadend_register_script (CString script);
extern int deadend_execute_scripts (int continue_on_failure);
extern void deadend_script_init (void);

extern const char *DEADENDS_search_path;
extern const char *GEDCOM_search_path;
extern const char *PYTHON_search_path;
