extern void llgettext_set_default_localedir (CString localeDir);
extern void llgettext_init (CString domain, CString codeset);
extern void llgettext_term (void);
extern void update_textdomain_localedir (CString domain, CString prefix);
extern void ll_bindtextdomain (CString domain, CString localeDir);
extern void init_win32_gettext_shim (void);
extern void set_gettext_codeset (CString domain, CString codeset);
extern CString get_gettext_codeset (void);
