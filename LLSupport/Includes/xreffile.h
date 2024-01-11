extern void initxref (void);

extern STRING getfxref (void);
extern STRING getsxref (void);
extern STRING getexref (void);
extern STRING getxxref (void);

extern INT32 getixrefnum (void);
extern void dumpxrefs (void);

extern void addixref (INT key);
extern void addfxref (INT key);
extern void addsxref (INT key);
extern void addexref (INT key);
extern void addxxref (INT key);

extern void addxref (CNSTRING key);
extern BOOLEAN addxref_if_missing (CNSTRING key);
extern BOOLEAN delete_xref_if_present (CNSTRING key);

extern BOOLEAN is_key_in_use (CNSTRING key);

extern INT xref_num_indis (void);
extern INT xref_num_fams (void); 
extern INT xref_num_sours (void);
extern INT xref_num_evens (void);
extern INT xref_num_othrs (void);

extern INT32 xref_max_indis (void);
extern INT32 xref_max_fams (void);
extern INT32 xref_max_sours (void);
extern INT32 xref_max_evens (void);
extern INT32 xref_max_othrs (void);

extern INT32 xref_max_any (void);

extern STRING newixref (STRING xrefp, BOOLEAN flag);
extern STRING newfxref (STRING xrefp, BOOLEAN flag);
extern STRING newsxref (STRING xrefp, BOOLEAN flag);
extern STRING newexref (STRING xrefp, BOOLEAN flag);
extern STRING newxxref (STRING xrefp, BOOLEAN flag);

extern INT xref_nexti (INT i);
extern INT xref_nextf (INT i);
extern INT xref_nexts (INT i);
extern INT xref_nexte (INT i);
extern INT xref_nextx (INT i);
extern INT xref_next (char ntype, INT i);

extern INT xref_previ (INT i);
extern INT xref_prevf (INT i);
extern INT xref_prevs (INT i);
extern INT xref_preve (INT i);
extern INT xref_prevx (INT i);
extern INT xref_prev (char ntype, INT i);

extern INT xref_firsti (void);
extern INT xref_firstf (void);
extern INT xref_firsts (void);
extern INT xref_firste (void);
extern INT xref_firstx (void);

extern INT xref_lasti (void);
extern INT xref_lastf (void);
extern INT xref_lasts (void);
extern INT xref_laste (void);
extern INT xref_lastx (void);
