/* ll-porting.h -- the purpose of this file is to ease porting between
   LifeLines and DeadEnds */

typedef void *		VPTR;

#define STRING		String

#define key_of_record(node)	nxref(node) /* XXX revisit when REFNs are supported XXX */

//#define store_record(key, rec, len)			storeRecord(
//#define choose_from_indiseq(seq, ask1, titl1, titln)	chooseFromSequence(seq, ask1, titl1, titln)

//#define edit_add_event()
//#define edit_add_source
//#define edit_add_other

#define create_node(xref, tag, val, prnt)	createGNode(xref, tag, val, prnt)
#define string_to_node(str)			stringToNodeTree(str)
#define node_to_string(node)			gnodesToString(node)
#define free_node(node,msg)			freeGNode(node)
#define free_nodes(node)			freeGNodes(node)
#define find_tag(node, tag)			findTag(node, tag)

#define ARRSIZE(array)		ARRAYSIZE(array)

#define key_to_record(key)	__llpy_key_to_record(key, NULL, database)
#define qkey_to_record(key)	__llpy_key_to_record(key, NULL, database)

#define key_possible_to_record(key, letter)	__llpy_key_to_record(key, NULL, database)
#define key_to_fam(key)		keyToFamily(key, database)
#define key_to_indi(key)	keyToPerson(key, database)
#define qkey_to_indi(key)	keyToPerson(key, database)

#define INDISEQ		Sequence *
//#define ISize(seq)	((seq)->size)
#define ISize(seq)	((seq)->block.length)
#define length_indiseq(seq)	ISize(seq)

#define remove_indiseq(seq)	deleteSequence(seq)

/* remove an element from a sequence */
#define delete_indiseq(seq,key,name,index) removeFromSequence(seq,key,name,index)

#define copy_indiseq(seq)	copySequence(seq)
#define create_indiseq_null()	createSequence(database)

#define rename_indiseq(seq, key)			renameSequence(seq, key)
#define element_indiseq(seq, index, pkey, pname)	elementSequence(seq, index, pkey, pname)
#define str_to_indiseq(name, ctype)	stringToSequence(name, database)

/* elt is an SequenceEl (SORTEL) */
#define element_skey(elt)		(elt->key)

#define fam_to_children(node)		familyToChildren(node, database)
#define fam_to_fathers(node)		familyToFathers(node, database)
#define fam_to_mothers(node)		familyToMothers(node, database)
//#define fam_to_husb(node)		familyToHusband(node, database)
#define fam_to_first_chil(node)		familyToFirstChild(node, database)
#define fam_to_key(fam)			familyToKey(fam)

#define indi_to_children(node)		personToChildren(node, database)
#define indi_to_families(person, fams)	personToFamilies(person, fams, database)
#define indi_to_fathers(node)		personToFathers(node, database)
#define indi_to_mothers(node)		personToMothers(node, database)
#define indi_to_spouses(node)		personToSpouses(node, database)

#define indi_to_name(node, len)		personToName(node,len)
#define indi_to_title(node, len)	personToTitle(node,len)

#define split_indi_old(indi,name,refn,sex,body,famc,fams) splitPerson(indi,name,refn,sex,body,famc,fams)
#define split_fam(fam,refn,husb,wife,chil,rest) splitFamily(fam,refn,husb,wife,chil,rest)
#define join_indi(indi,name,refn,sex,body,famc,fams) joinPerson(indi,name,refn,sex,body,famc,fams)
#define join_fam(fam,refn,husb,wife,chil,rest) joinFamily(fam,refn,husb,wife,chil,rest)
#define split_othr(othr,refn,body)	splitOther(othr,refn,body)
#define join_othr(othr,refn,body)	joinOther(othr,refn,body)

#define fam_to_cache(node)	/* empty */
#define even_to_cache(node)	/* empty */
#define othr_to_cache(node)	/* empty */
#define sour_to_cache(node)	/* empty */

#define indi_to_dbase(node)	addOrUpdatePersonInDatabase(node, database)
#define fam_to_dbase(node)	addOrUpdateFamilyInDatabase(node, database)
#define even_to_dbase(node)	addOrUpdateEventInDatabase(node, database)
#define othr_to_dbase(node)	addOrUpdateOtherInDatabase(node, database)
#define sour_to_dbase(node)	addOrUpdateSourceInDatabase(node, database)

#define fam_to_cacheel(record)	0
#define indi_to_cacheel(record)	0
#define lock_cache(cel)		/* empty */
#define unlock_cache(cel)	/* empty */

#define uchar		u_char
#define INT32		int32_t
#define INT16		int16_t

#define TABLE		HashTable *
#define create_table_obj()			createHashTable(NULL, NULL, NULL)
#define destroy_table(table)			deleteHashTable(table)
//#define insert_table_obj(table, element)	insertInHashTable(table, element)

#define create_table_str()			createStringTable()
#define insert_table_str(table, key, value)	insertInStringTable(table,key,value)
#define replace_table_str(table, key, value)	insertInStringTable(table,key,value)
#define valueof_str(table, key)			searchStringTable(table,key)

#define create_table_int()			createIntegerTable()
#define insert_table_int(table, key, value)	insertInIntegerTable(table,key,value)
#define valueof_int(table, key)			searchIntegerTable(table,key)
#define valueof_obj(table, key)			searchHashTable(table, key)

#define release_table(table)			releaseHashTable(table)
#define addref_table(table)			addrefHashTable(table)

#define LIST		List *
#define create_list()			createList(NULL, NULL, NULL, false)
#define enqueue_list			enqueueList

#define back_list(list, elt)		appendListElement(list, elt)
#define destroy_list(list)		deleteList(list)
#define is_empty_list(list)		isEmptyList(list)
#define length_list(list)		lengthList(list)
#define pop_list(list)			getAndRemoveFirstListElement(list)
#define push_list(list, vptr)		prependToList(list, vptr)

#define num_indis()			numberPersons(database)
#define num_fams()			numberFamilies(database)
#define num_sours()			numberSources(database)
#define num_evens()			numberEvents(database)
#define num_othrs()			numberOthers(database)

#define node_to_key(node)	(rmvat(nxref(node)))
#define normalize_rec(rec)	normalizeNodeTree(rec->root)

#define HINT_PARAM_UNUSED		ATTRIBUTE_UNUSED
#define HINT_PRINTF(fmt, args)		ATTRIBUTE_PRINTF(fmt, args)

#define FORINDISEQ(seq,elt,ndx)		FORSEQUENCE(seq,elt,ndx)
#define ENDINDISEQ			ENDSEQUENCE

#ifdef WIN32
#define path_match(path1, path2)	(!stricmp(path1, path2))
#else
#define path_match(path1, path2)	(!strcmp(path1, path2))
#endif

//#define strfree(ptr_to_str)	{ stdfree (*ptr_to_str); *ptr_to_str = 0; }
#define ISNULL(k)		(!k || *k == 0)

#define resolve_refn_links(node)	resolveRefnLinks(node, database)
#define add_refn(refn, key)		addRefn(refn, key, database)
#define remove_refn(refn, key)		removeRefn(refn, key, database)
#define get_refn(refn)			getRefn(refn, database)
#define index_by_refn(node)		indexByRefn(node, database)
//#define annotate_with_supplemental(node, rfmt)	annotateWithSupplemental(node, rfmt, database)
#define traverse_refns(func, param)	traverseRefns(func, param, database)

/* experimental NKEY stuff */

#define NKEY	String

#define nkey_copy(src, dest)	*dest = strdup(*src)
#define nkey_eq(nkey1, nkey2)	strcmp (*nkey1, *nkey2)
#define nkey_clear(nkey)	*nkey = 0
#define nkey_zero()		0
