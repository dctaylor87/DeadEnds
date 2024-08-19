#define DATE_STR_LEN	21	/* "%04d-%02d-%02d-%02d:%02d:%02dZ" */

extern void crashlog (String fmt, ...);
extern void crashlogn (String fmt, ...);
extern void errlog_out(CString title, CString msg, CString file, int line);
extern void crash_setcrashlog (String crashlog);
extern void crash_setdb (String dbname);
