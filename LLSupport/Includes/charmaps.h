extern int get_decimal (String str);
extern int get_hexidecimal (String str);
extern int hexvalue (int c);

extern ZSTR custom_translate (CString str, TRANTABLE tt);
extern void custom_translatez (ZSTR zstr, TRANTABLE tt);

extern bool init_map_from_file (CString file, CString mapname,
				TRANTABLE * ptt, ZSTR zerr);
