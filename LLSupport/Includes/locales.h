extern STRING get_original_locale_collate (void);
extern STRING get_current_locale_collate (void);
extern STRING get_original_locale_msgs (void);
extern STRING get_current_locale_msgs (void);
extern BOOLEAN are_locales_supported (void);
extern BOOLEAN is_nls_supported (void);
extern BOOLEAN is_iconv_supported (void);
extern STRING ll_langinfo (void);
extern void termlocale (void);
extern void uilocale (void);
extern void rptlocale (void);
extern STRING rpt_setlocale (STRING str);

extern void locales_notify_uicodeset_changes (void);
extern void locales_notify_language_change (void);
