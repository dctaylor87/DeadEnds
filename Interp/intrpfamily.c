//
//  intrpfamily.c
//  JustParsing
//
//  Created by Thomas Wetmore on 17 March 2023.
//  Last changed on 14 November 2023.
//

#include "standard.h"
#include "pnode.h"
#include "pvalue.h"
#include "evaluate.h"
#include "lineage.h"
#include "database.h"

//extern RecordIndex *theIndex;
extern Database *theDatabase;

//  __marriage -- Return the first marriage event of a family.
//    usage: marriage(FAM) -> EVENT
//--------------------------------------------------------------------------------------------------
PValue __marriage (PNode *pnode, SymbolTable *symtab, bool* errflg)
{
	ASSERT(pnode && symtab);
	GNode* fam = evaluateFamily(pnode->arguments, symtab, errflg);
	if (*errflg || !fam) return nullPValue;
	GNode* event = MARR(fam);
	return event ? PVALUE(PVEvent, uGNode, event) : nullPValue;
}

//  __husband -- Find the first husband of a family.
//    usage: husband(FAM) -> INDI
//--------------------------------------------------------------------------------------------------
PValue __husband(PNode *pnode, SymbolTable *symtab, bool* errflg)
{
	ASSERT(pnode && symtab);
	GNode* fam = evaluateFamily(pnode->arguments, symtab, errflg);
	if (*errflg || !fam) return nullPValue;
	GNode* husband = familyToHusband(fam, theDatabase);
	if (!husband) return nullPValue;
	return PVALUE(PVPerson, uGNode, husband);
}

//__wife -- Find the first wife of a family.
//    usage: wife(FAM) -> INDI
//--------------------------------------------------------------------------------------------------
PValue __wife(PNode *pnode, SymbolTable *symtab, bool* errflg)
{
	ASSERT(pnode && symtab);
	GNode* fam = evaluateFamily(pnode->arguments, symtab, errflg);
	if (*errflg || !fam) return nullPValue;
	GNode* wife = familyToWife(fam, theDatabase);
	if (!wife) return nullPValue;
	return PVALUE(PVPerson, uGNode, wife);
}

//  __nchildren -- Find the number of children in a family.
//    usage: nchildren(FAM) -> INT
//--------------------------------------------------------------------------------------------------
PValue __nchildren (PNode *pnode, SymbolTable *symtab, bool* eflg)
{
	ASSERT(pnode && symtab);
	GNode* fam = evaluateFamily(pnode->arguments, symtab, eflg);
	if (*eflg || !fam) return nullPValue;
	int count = 0;
	GNode* this = CHIL(fam);
	while (this && eqstr("CHIL", this->tag)) {
		count++;
		this = this->sibling;
	}
	return PVALUE(PVInt, uInt, (long) count);
}


//  __firstchild -- Return the first child of a family.
//    usage: firstchild(FAM) -> INDI
//--------------------------------------------------------------------------------------------------
PValue __firstchild(PNode *pnode, SymbolTable *symtab, bool* eflg)
{
	ASSERT(pnode && symtab);
	GNode* fam = evaluateFamily(pnode->arguments, symtab, eflg);
	if (*eflg || !fam) return nullPValue;
	GNode* child = familyToFirstChild(fam, theDatabase);
	if (!child) return nullPValue;
	return PVALUE(PVPerson, uGNode, child);
}

//  __lastchild -- Return the last child of a family.
//    usage: lastchild(FAM) -> INDI
//--------------------------------------------------------------------------------------------------
PValue __lastchild(PNode *pnode, SymbolTable *symtab, bool* eflg)
{
	ASSERT(pnode && symtab);
	GNode* fam = evaluateFamily(pnode->arguments, symtab, eflg);
	if (*eflg || !fam) return nullPValue;
	GNode* child = familyToLastChild(fam, theDatabase);
	if (!child) return nullPValue;
	return PVALUE(PVPerson, uGNode, child);
}

///*================================
// * __fnode -- Return root of family
// *   usage: fnode(FAM) -> NODE
// *==============================*/
//WORD __fnode (node, stab, eflg)
//INTERP node; TABLE stab; bool *eflg;
//{
//    return (WORD) eval_fam(ielist(node), stab, eflg, NULL);
//}

//  __fam -- Convert a key to FAM root node.
//    usage: fam(STRING) -> FAM
//--------------------------------------------------------------------------------------------------
PValue __fam(PNode *pnode, SymbolTable *symtab, bool* eflg)
{
	ASSERT(pnode && symtab);
	// The argument must be a string.
	PValue value = evaluate(pnode->arguments, symtab, eflg);
	if (value.type != PVString) {
		printf("the argument must be a string\n");
		*eflg = true;
		return nullPValue;
	}
	String key = value.value.uString;

	//  Search the database for the family with the key.
	GNode* family = keyToFamily(key, theDatabase);
	if (!family) {
		printf("Could not find a family with key %s.\n", key);
		return nullPValue;
	}
	return PVALUE(PVFamily, uGNode, family);
}

/*=============================================
 * firstfam -- Return first family in database.
 *   usage: firstfam() -> FAM
 *===========================================*/
//WORD __firstfam (node, stab, eflg)
//INTERP node; TABLE stab; BOOLEAN *eflg;
//{
//    NODE fam;
//    static char key[10];
//    STRING record;
//    INT len, i = 0;
//    *eflg = FALSE;
//    while (TRUE) {
//        sprintf(key, "F%d", ++i);
//        if (!(record = retrieve_record(key, &len)))
//            return NULL;
//        if (!(fam = stringToNodeTree(record))) {
//            stdfree(record);
//            continue;
//        }
//        stdfree(record);
//        free_nodes(fam);/*yes*/
//        return (WORD) fam_to_cacheel(fam);
//    }
//}
/*===========================================
 * nextfam -- Return next family in database.
 *   usage: nextfam(FAM) -> FAM
 *=========================================*/
//WORD __nextfam (node, stab, eflg)
//INTERP node; TABLE stab; BOOLEAN *eflg;
//{
//    NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
//    static char key[10];
//    STRING record;
//    INT len, i;
//    if (*eflg) return NULL;
//    strcpy(key, fam_to_key(fam));
//    i = atoi(&key[1]);
//    while (TRUE) {
//        sprintf(key, "F%d", ++i);
//        if (!(record = retrieve_record(key, &len)))
//            return NULL;
//        if (!(fam = stringToNodeTree(record))) {
//            stdfree(record);
//            continue;
//        }
//        stdfree(record);
//        free_nodes(fam);/*yes*/
//        return (WORD) fam_to_cacheel(fam);
//    }
//}
/*===============================================
 * prevfam -- Return previous family in database.
 *   usage: prevfam(FAM) -> FAM
 *=============================================*/
//WORD __prevfam (node, stab, eflg)
//INTERP node; TABLE stab; BOOLEAN *eflg;
//{
//    NODE fam = eval_fam(ielist(node), stab, eflg, NULL);
//    static char key[10];
//    STRING record;
//    INT len, i;
//    if (*eflg) return NULL;
//    strcpy(key, fam_to_key(fam));
//    i = atoi(&key[1]);
//    while (TRUE) {
//        sprintf(key, "F%d", --i);
//        if (!(record = retrieve_record(key, &len)))
//            return NULL;
//        if (!(fam = stringToNodeTree(record))) {
//            stdfree(record);
//            continue;
//        }
//        stdfree(record);
//        free_nodes(fam);/*yes*/
//        return (WORD) fam_to_cacheel(fam);
//    }
//}
/*============================================
 * lastfam -- Return last family in database.
 *   usage: lastfam() -> FAM
 *==========================================*/
//WORD __lastfam (node, stab, eflg)
//INTERP node; TABLE stab; BOOLEAN *eflg;
//{
//}
