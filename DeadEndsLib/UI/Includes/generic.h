/* generic.h -- prototypes of generic, unique, UI functions */

/* forward reference */
enum SequenceType;

int ask_for_char(CString ttl, CString prmpt, CString ptrn);
bool ask_for_db_filename (CString ttl, CString prmpt, CString basedir, String buffer, int buflen);
int chooseFromList (CString ttl, List *list);
int chooseListFromSequence (CString ttl, Sequence *seq, enum SequenceType type);
int chooseOneFromSequence (CString ttl, Sequence *seq, enum SequenceType type);
bool yes_no_value(int c);
