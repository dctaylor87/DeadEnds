extern RecordIndexEl *ask_for_fam (CString pttl, CString sttl);
extern RecordIndexEl *ask_for_fam_by_key(CString fttl, CString pttl, CString sttl);
extern RecordIndexEl *ask_for_indi (CString ttl, ASK1Q ask1);
extern String ask_for_indi_key (CString ttl, ASK1Q ask1);
extern Sequence *ask_for_indi_list (CString ttl, bool reask);
extern bool ask_for_int (CString ttl, int *prtn);
extern FILE *ask_for_input_file (CString mode,
				 CString ttl,
				 String *pfname,
				 String *pfullpath,
				 CString path,
				 CString ext);
extern FILE *ask_for_output_file (CString mode,
				  CString ttl,
				  String *pfname,
				  String *pfullpath,
				  CString path,
				  CString ext);
