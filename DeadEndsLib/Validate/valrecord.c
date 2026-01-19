//
//  DeadEnds Library
//
// valrecord.c has code to validate a record against an existing database.

#include <stdint.h>

#include "errors.h"
#include "gnode.h"
#include "rootlist.h"
#include "gedcom.h"
#include "hashtable.h"
#include "recordindex.h"
#include "refnindex.h"
#include "database.h"
#include "validate.h"
#include "denls.h"
#include "splitjoin.h"
#include "nodeutils.h"
#include "refns.h"
#include "messages.h"

static bool valid_name (String name);

/* validateRecord -- validate 'record' against pre-existing database
   'database'.

   If validation succeeds, true is returned.  Otherwise, false is
   returned and the errors are recorded in errorLog.  */

bool validateRecord (GNode *record,
		     Database *database,
		     ErrorLog *errorLog)
{
  bool errorsFound = false;

  FORTRAVERSE(record, node)
    if (isKey(nval(node)))
      {
	/* first, check if the key is found in recordIndex */
	if (! searchRecordIndex (database->recordIndex, nval(node)))
	  {
	    /* key not found, give bad news */
	    CString msg = "tag '%s' within record '%s' has invalid key '%s'";
	    int len = strlen(msg) + strlen(ntag(node))
	      + (nxref(record) ? strlen(nxref(record)) : 0)
	      + strlen(nval(node)) + 1;
	    char message[len];
	    snprintf(message, len, msg, ntag(node),
		     (nxref(record) ? nxref(record) : ""), nval(node));
	    Error *error = createError(linkageError, database->name, 0, message);
	    addErrorToLog (errorLog, error);
	    errorsFound = true;
	    continue;
	  }

	/* next, check if the key is in the corresponding root index */
	/* only check the idividual indexes if the key exists */
	RootList *root = null;
	CString msg = null;
	switch (nval(node)[1])
	  {
	  case 'I':
	    root = database->personRoots;
	    msg = "tag '%s' within record '%s' has key '%s' which is missing from personRoots";
	    break;
	  case 'F':
	    root = database->familyRoots;
	    msg = "tag '%s' within record '%s' has key '%s' which is missing from familyRoots";
	    break;
	  case 'S':
	    root = database->sourceRoots;
	    msg = "tag '%s' within record '%s' has key '%s' which is missing from sourceRoots";
	    break;
	  case 'E':
	    root = database->eventRoots;
	    msg = "tag '%s' within record '%s' has key '%s' which is missing from eventRoots";
	    break;
	  case 'X':
	    root = database->otherRoots;
	    msg = "tag '%s' within record '%s' has key '%s' which is missing from otherRoots";
	    break;
	  default:
	    root = null;
	    msg = "tag '%s' within record '%s' has key '%s' which is of unknown type";
	    break;
	  }

	if (root)
	  {
	    if (! isInList (root, nval(node), null))
	      {
		/* key not found, give bad news */
		int len = strlen(msg) + strlen(ntag(node))
		  + (nxref(record) ? strlen(nxref(record)) : 0)
		  + strlen(nval(node)) + 1;
		char message[len];
		snprintf(message, len, msg, ntag(node), nxref(record), nval(node));
		Error *error = createError(linkageError, database->name, 0, message);
		addErrorToLog (errorLog, error);
		errorsFound = true;
	      }
	  }
	else
	  {
	    /* unrecognized key type, give bad news */
	    int len = strlen(msg) + strlen(ntag(node))
	      + (nxref(record) ? strlen(nxref(record)) : 0) + 1;
	    char message[len];
	    snprintf(message, len, msg, ntag(node), nxref(record));
	    Error *error = createError(linkageError, database->name, 0, message);
	    addErrorToLog (errorLog, error);
	    errorsFound = true;
	  }
      }
  ENDTRAVERSE
  return (! errorsFound);
}

/* validateNewPerson -- Validate person tree

   indi1:     [IN]  person to validate
   orig:      [IN]  person to match - may be NULL
   database:  [IN]  
   errorLog: [OUT] error message(s), if any
   rtn: false for bad
*/

bool
validateNewPerson (GNode *indi1, GNode *orig, Database *database, ErrorLog *errorLog)
{
	GNode *refn;
	GNode *name1, *refn1, *sex1, *body1, *famc1, *fams1, *node;
	GNode *name0, *refn0, *sex0, *body0, *famc0, *fams0;
	SexType isex;
	String ukey;

	if (!indi1) {
		Error *error = createError (gedcomError, database->name, 0, _(qSbademp));
		addErrorToLog (errorLog, error);
  		return false;
	}
	if (nestr("INDI", ntag(indi1))) {
		Error *error = createError (gedcomError, database->name, 0, _(qSbadin0));
		addErrorToLog (errorLog, error);
		return false;
	}
	if (nsibling(indi1)) {
		Error *error = createError (syntaxError, database->name, 0, _(qSbadmul));
		addErrorToLog (errorLog, error);
		return false;
	}
	splitPerson(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	if (getdeoptint("RequireNames", 0) && !name1) {
		Error *error = createError (gedcomError, database->name, 0,
					    _("This person record does not have a name line."));
		addErrorToLog (errorLog, error);
		goto bad2;
	}
	for (node = name1; node; node = nsibling(node)) {
		if (!valid_name(nval(node))) {
			Error *error = createError (syntaxError, database->name, 0, _(qSbadenm));
			addErrorToLog (errorLog, error);
			goto bad2;
		}
	}
	name0 = refn0 = sex0 = body0 = famc0 = fams0 = NULL;
	if (orig)
		splitPerson(orig, &name0, &refn0, &sex0, &body0, &famc0,
		    &fams0);
	if (orig && !isoGNodes(indi1, orig, false, false)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadind));
		addErrorToLog (errorLog, error);
		goto bad1;
	}
	if (!isoGNodes(famc1, famc0, false, true)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadfmc));
		addErrorToLog (errorLog, error);
		goto bad1;
	}
	if (!isoGNodes(fams1, fams0, false, true)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadfms));
		addErrorToLog (errorLog, error);
		goto bad1;
	}
	isex = valueToSex(sex0);
	if (!fams0) isex = sexUnknown;
	if (isex != sexUnknown && isex != valueToSex(sex1)) {
		Error *error = createError (gedcomError, database->name, 0, _(qSbadparsex));
		addErrorToLog (errorLog, error);
		goto bad1;
	}
	/* if there is more than one record with the REFN, then the
	   database is broken -- while a record can have an arbitrary
	   number of REFNs, each REFN *MUST* be unique. */
	if (database) {
	  /* we can only check REFNs if we have a database */
	  for (refn = refn1; refn != NULL; refn = nsibling(refn)) {
	    ukey = nval(refn);
	    CString key = getRefn (ukey, database);
	    if (! key || ! orig || nestr(key, nxref(indi1))) {
	      Error *error = createError (gedcomError, database->name, 0, _(qSbadirefn));
	      addErrorToLog (errorLog, error);
	      goto bad1;
	    }
	  }
	}
	if (orig)
		joinPerson(orig, name0, refn0, sex0, body0, famc0, fams0);
	joinPerson(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return true;
bad1:
	if (orig)
		joinPerson(orig, name0, refn0, sex0, body0, famc0, fams0);
bad2:
	joinPerson(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return false;
}

/* validateNewFamily -- Validate FAM tree

   fam1,     [IN]  family to validate
   fam0:     [IN]  family to match - may be NULL
   database: [IN]
   errorLog: [OUT]
*/

bool
validateNewFamily (GNode *fam1, GNode *fam0,
		Database *database, ErrorLog *errorLog)
{
	GNode *refn0, *husb0, *wife0, *chil0, *body0;
	GNode *refn1, *husb1, *wife1, *chil1, *body1;

	if (!fam1) {
		Error *error = createError (gedcomError, database->name, 0, _(qSbademp));
		addErrorToLog (errorLog, error);
  		return false;
	}
	if (nestr("FAM", ntag(fam1))) {
		Error *error = createError (gedcomError, database->name, 0, _(qSbadfm0));
		addErrorToLog (errorLog, error);
		return false;
	}
	if (nsibling(fam1)) {
		Error *error = createError (syntaxError, database->name, 0, _(qSbadmul));
		addErrorToLog (errorLog, error);
		return false;
	}

	refn0 = husb0 = wife0 = chil0 = body0 = NULL;
	if (fam0)
		splitFamily(fam0, &refn0, &husb0, &wife0, &chil0, &body0);
	splitFamily(fam1, &refn1, &husb1, &wife1, &chil1, &body1);
	
	if (fam0 && !isoGNodes(fam1, fam0, false, true)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadfam));
		addErrorToLog (errorLog, error);
		goto bad3;
	}
	if (!isoGNodes(husb1, husb0, false, true)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadhsb));
		addErrorToLog (errorLog, error);
		goto bad3;
	}
	if (!isoGNodes(wife1, wife0, false, true)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadwif));
		addErrorToLog (errorLog, error);
		goto bad3;
	}
	if (!isoGNodes(chil1, chil0, false, true)) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadchl));
		addErrorToLog (errorLog, error);
		goto bad3;
	}
	if (fam0)
		joinFamily(fam0, refn0, husb0, wife0, chil0, body0);
	joinFamily(fam1, refn1, husb1, wife1, chil1, body1);
	return true;
bad3:
	if (fam0)
		joinFamily(fam0, refn0, husb0, wife0, chil0, body0);
	joinFamily(fam1, refn1, husb1, wife1, chil1, body1);
	return false;
}

/*============================
 * valid_name -- Validate name
 *==========================*/
static bool
valid_name (String name)
{
	int c, n = 0;
	if (!name) return false;
	if (isKey(name)) return false;
	while ((c = *name++)) {
		if (c == '/') n++; /* '/' was NAMESEP */
	}
	return n <= 2;
}

/* validateNewRecord -- Validate top-level node tree.

   NOTE: assumes database is already complete and validated.  This is
   used to validate new records and modifications of existing records.

   node:     [IN]  node to validate
   ntype:    [IN]  RecordType (think: I/F/S/E/X)
   orig:     [IN]  old node to match (may be null)
   database: [IN]
   errorLog: [OUT]
*/

bool
validateNewRecord (GNode *node,	/* modified tree, to be validated */
		   GNode *node0, /* original tree */
		   RecordType ntype,
		   Database *database,
		   ErrorLog *errorLog)
{
  bool retval;

  switch(ntype)
    {
    case GRPerson:
      retval = validateNewPerson(node, node0, database, errorLog);
      break;
    case GRFamily:
      retval = validateNewFamily(node, node0, database, errorLog);
      break;
    case GRSource:
      retval = validateNewSource(node, node0, database, errorLog);
      break;
    case GREvent:
      retval = validateNewEvent(node, node0, database, errorLog);
      break;
    default:
      retval = validateNewOther(node, node0, database, errorLog);
      break;
    }

  if (! retval)
    return (retval);

  retval = validateRecord (node, database, errorLog);
  return (retval);
}

/*======================================
 * validateNewSource -- Validate SOUR tree
 *  node:  [IN]  source to validate 
 *  pmsg:  [OUT] error message, if any 
 *  orig:  [IN]  SOUR node to match 
 *====================================*/
bool
validateNewSource (GNode *node, ATTRIBUTE_UNUSED GNode *orig,
		   Database *database, ErrorLog *errorLog)
{
	if (!node) {
		Error *error = createError (gedcomError, database->name, 0, _(qSbademp));
		addErrorToLog (errorLog, error);
  		return false;
	}
	if (nestr("SOUR", ntag(node))) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadsr0));
		addErrorToLog (errorLog, error);
		return false;
	}
#if 0
	/* validation unimplemented */
	if (orig)
	{
		Error *error = createError (linkageError, database->name, 0, _(qSbadsr0));
		addErrorToLog (errorLog, error);
		return false;
	}
#endif
	return true;
}

/*======================================
 * validateNewEvent -- Validate EVEN tree
 *  node:  [IN]  source to validate
 *  pmsg,  [OUT] error message, if any
 *  orig:  [IN]  EVEN node to match
 *====================================*/
bool
validateNewEvent (GNode *node, ATTRIBUTE_UNUSED GNode *orig,
		  Database *database, ErrorLog *errorLog)
{
	if (!node) {
		Error *error = createError (linkageError, database->name, 0, _(qSbademp));
		addErrorToLog (errorLog, error);
  		return false;
	}
	if (nestr("EVEN", ntag(node))) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadev0));
		addErrorToLog (errorLog, error);
		return false;
	}
#if 0
	/* validation unimplemented */
	if (orig)
	{
		Error *error = createError (linkageError, database->name, 0, _(qSbadev0));
		addErrorToLog (errorLog, error);
		return false;
	}
#endif
	return true;
}
/*======================================
 * validateNewOther -- Validate OTHR tree
 *  node:  [IN]  source to validate
 *  pmsg,  [OUT] error message, if any
 *  orig:  [IN]  OTHR node to match
 *====================================*/
bool
validateNewOther (GNode *node, ATTRIBUTE_UNUSED GNode *orig,
		  Database *database, ErrorLog *errorLog)
{
	if (!node) {
		Error *error = createError (linkageError, database->name, 0, _(qSbademp));
		addErrorToLog (errorLog, error);
  		return false;
	}
	if (eqstr("INDI", ntag(node)) || eqstr("FAM", ntag(node))
		|| eqstr("SOUR", ntag(node)) || eqstr("EVEN", ntag(node))) {
		Error *error = createError (linkageError, database->name, 0, _(qSbadothr0));
		addErrorToLog (errorLog, error);
		return false;
	}
#if 0
	/* validation unimplemented */
	if (orig)
	{
		Error *error = createError (linkageError, database->name, 0, _(qSbadothr0));
		addErrorToLog (errorLog, error);
		return false;
	}
#endif
	return true;
}
