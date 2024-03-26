extern void initxref (void);

extern String getfxref (void);
extern String getsxref (void);
extern String getexref (void);
extern String getxxref (void);

extern INT32 getixrefnum (void);
extern void dumpxrefs (void);

extern void addixref (int key);
extern void addfxref (int key);
extern void addsxref (int key);
extern void addexref (int key);
extern void addxxref (int key);

extern void addxref (CString key);
extern bool addxref_if_missing (CString key);
extern bool delete_xref_if_present (CString key);

extern bool is_key_in_use (CString key);

extern int xref_num_indis (void);
extern int xref_num_fams (void); 
extern int xref_num_sours (void);
extern int xref_num_evens (void);
extern int xref_num_othrs (void);

extern INT32 xref_max_indis (void);
extern INT32 xref_max_fams (void);
extern INT32 xref_max_sours (void);
extern INT32 xref_max_evens (void);
extern INT32 xref_max_othrs (void);

extern INT32 xref_max_any (void);

extern String newixref (String xrefp, bool flag);
extern String newfxref (String xrefp, bool flag);
extern String newsxref (String xrefp, bool flag);
extern String newexref (String xrefp, bool flag);
extern String newxxref (String xrefp, bool flag);

extern int xref_nexti (int i);
extern int xref_nextf (int i);
extern int xref_nexts (int i);
extern int xref_nexte (int i);
extern int xref_nextx (int i);
extern int xref_next (char ntype, int i);

extern int xref_previ (int i);
extern int xref_prevf (int i);
extern int xref_prevs (int i);
extern int xref_preve (int i);
extern int xref_prevx (int i);
extern int xref_prev (char ntype, int i);

extern int xref_firsti (void);
extern int xref_firstf (void);
extern int xref_firsts (void);
extern int xref_firste (void);
extern int xref_firstx (void);

extern int xref_lasti (void);
extern int xref_lastf (void);
extern int xref_lasts (void);
extern int xref_laste (void);
extern int xref_lastx (void);
