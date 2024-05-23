#ifndef listui_h_included
#define listui_h_included

INT array_interact (CString ttl, INT len, String *strings
	, bool selectable, DETAILFNC detfnc, void * param);

void listui_init_windows(INT extralines);
void listui_placecursor_main(INT * prow, INT * pcol);
void paint_list_screen(void);

#endif /* listui_h_included */
