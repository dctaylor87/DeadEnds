#ifndef charprops_h_included
#define charprops_h_included

bool charprops_load_utf8(void);
void charprops_free_all(void);
bool charprops_load(const char * codepage);
bool charprops_is_loaded(void);

#endif /* charprops_h_included */
