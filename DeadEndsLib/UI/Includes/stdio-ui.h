extern int stdio_ask_for_char_msg (CString msg, CString ttl,
				   CString prmpt, CString ptrn);
extern bool stdio_ask_for_filename_impl (CString ttl, CString path,
					 CString prmpt,
					 String buffer, int buflen);
extern bool stdio_ask_for_string (CString ttl, CString prmpt,
				  String buffer, int buflen);
extern bool stdio_ask_for_string2 (CString ttl1, CString ttl2, CString prmpt,
				   String buffer, int buflen);
extern int stdio_chooseFromArray (CString ttl, int no, String *pstrngs);
extern int stdio_prompt_stdout (CString prompt);
extern Sequence *stdio_invoke_search_menu (void);
extern void stdio_llvwprintf (CString fmt, va_list args);
extern void stdio_view_array (CString ttl, int no, String *pstrngs);
extern bool stdio_ask_for_program (CString mode, CString ttl, String *pfname,
				   CString path, CString ext, bool picklist);
extern int stdio_chooseOneOrListFromSequence (CString ttl, Sequence *seq,
					      bool multi, enum SequenceType type);
