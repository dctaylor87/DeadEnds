extern INT get_decimal (String str);
extern INT get_hexidecimal (String str);
extern INT hexvalue (INT c);

extern ZSTR custom_translate (CString str, TRANTABLE tt);
extern void custom_translatez (ZSTR zstr, TRANTABLE tt);

extern BOOLEAN init_map_from_file (CString file, CString mapname,
				   TRANTABLE * ptt, ZSTR zerr);
