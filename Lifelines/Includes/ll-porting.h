/* ll-porting.h -- the purpose of this file is to ease porting between
   LifeLines and DeadEnds */

typedef const char *	CNSTRING;
typedef void *		VPTR;

#define STRING		String

#define BOOLEAN		_Bool
#define NODE		GNode *

#define nchild(node)	((node)->child)
#define nsibling(node)	((node)->sibling)
#define nparent(node)	((node)->parent)
#define nval(node)	((node)->value)
#define ntag(node)	((node)->tag)
#define nxref(node)	((node)->key)

#define RECORD		RecordIndexEl *

#define nztop(record)	((record)->root)
#define nzkey(record)	((record)->root->key)
#define nztype(record)	((record)->root->key[0])

#define key_of_record(record)	nzkey(record) /* XXX revisit when REFNs are supported XXX */

//#define store_record(key, rec, len)			storeRecord(
//#define choose_from_indiseq(seq, ask1, titl1, titln)	chooseFromSequence(seq, ask1, titl1, titln)

#define create_indiseq_null()			createSequence(theDatabase)

//#define edit_add_event()
//#define edit_add_source
//#define edit_add_other

#define create_node(xref, tag, val, prnt)	createGNode(xref, tag, val, prnt)
#define string_to_node(str)			stringToNodeTree(str)
#define node_to_string(node)			gnodesToString(node)
#define free_nodes(node)			freeGNodes(node)

#define DOSURCAP	true
#define NOSURCAP	false
#define REGORDER	true
#define SURFIRST	false

#define DOASK1		1
#define NOASK1		0

#define ARRSIZE(array)		ARRAYSIZE(array)

#define release_record(record)		/* empty --= no current ref counts */

#define key_to_irecord(key)	keyToPersonRecord(key, theDatabase)
#define key_to_frecord(key)	keyToFamilyRecord(key, theDatabase)
#define key_to_srecord(key)	keyToSourceRecord(key, theDatabase)
#define key_to_erecord(key)	keyToEventRecord(key, theDatabase)
#define key_to_orecord(key)	keyToOtherRecord(key, theDatabase)
#define qkey_to_irecord(key)	keyToPersonRecord(key, theDatabase)
#define qkey_to_frecord(key)	keyToFamilyRecord(key, theDatabase)
#define qkey_to_srecord(key)	keyToSourceRecord(key, theDatabase)
#define qkey_to_erecord(key)	keyToEventRecord(key, theDatabase)
#define qkey_to_orecord(key)	keyToOtherRecord(key, theDatabase)

#define key_to_record(key)	__llpy_key_to_record(key, NULL)
#define qkey_to_record(key)	__llpy_key_to_record(key, NULL)

#define key_to_fam(key)		keyToFamily(key, theDatabase)
#define key_to_indi(key)	keyToPerson(key, theDatabase)

#define getlloptstr(property, default)	(getenv(property) ? getenv(property) : default) /* XXX */

#define INDISEQ		Sequence *
#define ISize(seq)	((seq)->size)
#define length_indiseq(seq)	ISize(seq)

/* XXX not sure if second argument should be 'false' or 'true' XXX */
#define remove_indiseq(seq)	deleteSequence(seq, false)

/* elt is an SequenceEl (SORTEL) */
#define element_skey(elt)		(elt->key)

#define fam_to_children(node)		familyToChildren(node, theDatabase)
#define fam_to_fathers(node)		familyToFathers(node, theDatabase)
#define fam_to_mothers(node)		familyToMothers(node, theDatabase)
#define fam_to_husb(node)		familyToHusband(node, theDatabase)
#define fam_to_first_chil(node)		familyToFirstChild(node, theDatabase)
#define fam_to_key(fam)			familyToKey(fam)

#define indi_to_children(node)		personToChildren(node, theDatabase)
#define indi_to_families(person, fams)	personToFamilies(person, fams, theDatabase)
#define indi_to_fathers(node)		personToFathers(node, theDatabase)
#define indi_to_mothers(node)		personToMothers(node, theDatabase)
#define indi_to_spouses(node)		personToSpouses(node, theDatabase)

#define indi_to_fath(node)		personToFather(node, theDatabase)
#define indi_to_moth(node)		personToMother(node, theDatabase)
#define indi_to_prev_sib(record)	personToPreviousSibling((record->root), theDatabase)
#define indi_to_next_sib(record)	personToNextSibling((record->root), theDatabase)

#define indi_to_name(node, len)		personToName(node,len)

#define split_indi_old(indi,name,refn,sex,body,famc,fams) splitPerson(indi,name,refn,sex,body,famc,fams)
#define split_fam(fam,refn,husb,wife,chil,rest) splitFamily(fam,refn,husb,wife,chil,rest)
#define join_indi(indi,name,refn,sex,body,famc,fams) joinPerson(indi,name,refn,sex,body,famc,fams)
#define join_fam(fam,refn,husb,wife,chil,rest) joinFamily(fam,refn,husb,wife,chil,rest)

#define fam_to_cache(node)	/* empty */
#define even_to_cache(node)	/* empty */
#define othr_to_cache(node)	/* empty */
#define sour_to_cache(node)	/* empty */

#define fam_to_dbase(node)	/* empty */
#define even_to_dbase(node)	/* empty */
#define othr_to_dbase(node)	/* empty */
#define sour_to_dbase(node)	/* empty */

#define fam_to_cacheel(record)	0
#define indi_to_cacheel(record)	0
#define lock_cache(cel)		/* empty */
#define unlock_cache(cel)	/* empty */

#define FALSE		false	/* <stdbool.h> */
#define TRUE		true	/* <stdbool.h> */

#define INT		int	/* XXX */
#define uchar		u_char
#define INT32		int32_t

#define TABLE		HashTable *
#define LIST		List *

#define create_list()			createList(NULL, NULL, NULL)

#define event_to_date(node,shorten)	eventToDate(node,shorten)
#define givens(name)			getGivenNames(name)
#define getasurname(name)		getSurname(name)
#define getsxsurname(name)		getSurname(name)
#define find_tag(node, tag)		findTag(node, tag)
#define trad_soundex(name)		soundex(name)

#define manip_name(name,caps,reg,len)	manipulateName(name,caps,reg,len)

#define name_string(str)		nameString(str)
#define trim_name(name,len)		trimName(name,len)

#define num_indis()			numberPersons(theDatabase)
#define num_fams()			numberFamilies(theDatabase)
#define num_sours()			numberSources(theDatabase)
#define num_evens()			numberEvents(theDatabase)
#define num_othrs()			numberOthers(theDatabase)

#define rmvat(key)		(key)

#define node_to_record(node)	_llpy_node_to_record(node)
#define node_to_key(node)	(rmvat(nxref(node)))
#define normalize_rec(rec)	normalizeNodeTree(rec->root)

/* we drop efmt -- era format */
#define do_format_date(str,dfmt,mfmt,yfmt,sfmt,efmt,cmplx) format_date(str,dfmt,mfmt,yfmt,sfmt,cmplx)

#define HINT_PARAM_UNUSED		ATTRIBUTE_UNUSED
#define HINT_PRINTF(fmt, args)		ATTRIBUTE_PRINTF(fmt, args)

#define FORINDISEQ(seq,elt,ndx)		FORSEQUENCE(seq,elt,ndx)
#define ENDINDISEQ			ENDSEQUENCE
