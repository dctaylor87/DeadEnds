typedef void (*CALLBACK_FNC)(void*);

extern String get_original_locale_collate (void);
extern String get_current_locale_collate (void);
extern String get_original_locale_msgs (void);
extern String get_current_locale_msgs (void);
extern bool are_locales_supported (void);
extern bool is_nls_supported (void);
extern bool is_iconv_supported (void);
extern String ll_langinfo (void);
extern void termlocale (void);
extern void uilocale (void);
extern void rptlocale (void);
extern String rpt_setlocale (String str);

extern void locales_notify_uicodeset_changes (void);
extern void locales_notify_language_change (void);

extern void save_original_locales (void);
extern char *llsetlocale (int category, char * locale);

extern void register_uilang_callback (CALLBACK_FNC fncptr, void* uparm);
extern void unregister_uilang_callback (CALLBACK_FNC fncptr, void* uparm);
extern void register_uicodeset_callback (CALLBACK_FNC fncptr, void* uparm);
extern void unregister_uicodeset_callback (CALLBACK_FNC fncptr, void* uparm);
