/*
  Functions used inside the liflines module
  requiring screen.h declarations (eg, UIWIN).
*/

#ifndef screeni_h_included
#define screeni_h_included

/* interact.c */
int interact_choice_string(UIWINDOW uiwin, CString str);
int interact_screen_menu(UIWINDOW uiwin, int screen);

/* screen.h */
void place_cursor_popup(UIWINDOW uiwin);

#endif /* screeni_h_included */
