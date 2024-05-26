#ifndef listui_h_included
#define listui_h_included

int array_interact (CString ttl, int len, String *strings
	, bool selectable, DETAILFNC detfnc, void * param);

void listui_init_windows(int extralines);
void listui_placecursor_main(int * prow, int * pcol);
void paint_list_screen(void);

#endif /* listui_h_included */
