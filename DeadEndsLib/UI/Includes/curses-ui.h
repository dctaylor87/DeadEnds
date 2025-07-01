extern int curses_ask_for_char_msg (CString msg, CString ttl,
				    CString prmpt, CString ptrn);
extern bool curses_ask_for_filename_impl (CString ttl, CString path,
					  CString prmpt,
					  String buffer, int buflen);
extern bool curses_ask_for_string (CString ttl, CString prmpt,
				   String buffer, int buflen);
extern bool curses_ask_for_string2 (CString ttl1, CString ttl2, CString prmpt,
				    String buffer, int buflen);
extern int curses_chooseFromArray (CString ttl, int no, String *pstrngs);
extern int curses_prompt_stdout (CString prompt);
extern Sequence curses_invoke_search_menu (void);
extern void curses_llvwprintf (CString fmt, va_list args);
extern void curses_view_array (CString ttl, int no, String *pstrngs);
extern bool curses_ask_for_program (CString mode, CString ttl, String *pfname,
				    CString path, CString ext, bool picklist);
enum SequenceType;		/* forward reference */
extern int curses_chooseOneOrListFromSequence (CString ttl, Sequence *seq,
					       bool multi, enum SequenceType type);

extern void
curses_outputv (void *data, MSG_LEVEL level, CString fmt, va_list args);
