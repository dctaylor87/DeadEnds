extern void llgettext_set_default_localedir (CNSTRING localeDir);
extern void llgettext_init (CNSTRING domain, CNSTRING codeset);
extern void llgettext_term (void);
extern void update_textdomain_localedir (CNSTRING domain, CNSTRING prefix);
extern void ll_bindtextdomain (CNSTRING domain, CNSTRING localeDir);
extern void init_win32_gettext_shim (void);
extern void set_gettext_codeset (CNSTRING domain, CNSTRING codeset);
extern CNSTRING get_gettext_codeset (void);
