#include <ansidecl.h>
#include <stdint.h>
#include <limits.h>		/* PATH_MAX */

#include "standard.h"
#include "hashtable.h"
#include "refnindex.h"
#include "errors.h"
#include "gnode.h"    // GNode.
#include "lineage.h"
#include "list.h"
#include "pvalue.h"   // PValue.
#include "pnode.h"    // PNode.
#include "name.h"     // getSurname, manipulateName.
#include "functiontable.h"
#include "interp.h"
#include "recordindex.h" // searchRecordIndex.
#include "database.h"    // personIndex, familyIndex.
#include "hashtable.h"
#include "evaluate.h"  // evaluate.
#include "path.h"      // fopenPath.
#include "symboltable.h"
#include "context.h"
#include "date.h"
#include "place.h"
#include "builtintable.h"
#include "sequence.h"
#include "gedcom.h"

#include "ask.h"
#include "rptui.h"
#include "denls.h"
#include "locales.h"
#include "lloptions.h"
#include "messages.h"
#include "de-strings.h"
#include "feedback.h"
#include "generic.h"
#include "ll-sequence.h"

#if 0				/* unused, Tom has now implemented __index */
static void compute_pi(String pi, CString sub);
static int ll_index(CString str, CString sub, int num);
static int kmp_search(CString pi, CString str, CString sub, int num);
#endif

// * __getindi -- Have user identify person
// *   usage: getindi(IDEN [,STRING]) --> VOID
// *   usage: getindimsg(IDEN, STRING) --> VOID
// *=========================================*/
PValue __getindi (PNode *pnode, Context *context, bool* eflg)
{
  PNode *iden = pnode->arguments;
  if (iden->type != PNIdent)
    {
      *eflg = true;
      return nullPValue;
    }
  PNode *expr = iden->next;
  String ttl = "Identify person for report program:";
  if (expr)
    {
      PValue svalue = evaluate (expr, context, eflg);
      if (*eflg)
	return nullPValue;
      if (svalue.type != PVString)
	{
	  *eflg = true;
	  return nullPValue;
	}
      ttl = svalue.value.uString;
    }
    
    CString key = rptui_ask_for_indi_key(ttl, DOASK1, currentDatabase); /* XXX */
    if (!key)
      return nullPValue;

    assignValueToSymbol(context, iden->identifier,
			PVALUE (PVPerson, uGNode,
				keyToPerson (key, context->database->recordIndex)));
    return nullPValue;
}

/* __choosechild -- have user choose child of person or family.
   usage: choosechild (INDI|FAM) --> INDI.  */

PValue __choosechild (PNode *pnode, Context *context, bool* errflg)
{
  PValue gnv;
  RecordType type;
  Database *database = context->database; /* XXX allow database as an optional 2nd argument XXX */

  gnv = evaluate (pnode->arguments, context, errflg);
  if (*errflg)
    {
      scriptError (pnode, "Error evaluating argument to choosechild.");
      return nullPValue;
    }
  if (gnv.type == PVPerson)
    type = GRPerson;
  else if (gnv.type == PVFamily)
    type = GRFamily;
  else if (gnv.type == PVGNode)
    {
      switch (recordType (gnv.value.uGNode))
	{
	case GRPerson:
	  type = GRPerson;
	  break;
	case GRFamily:
	  type = GRFamily;
	  break;
	default:
	  scriptError (pnode, "Argument to choosechild must be either a family or a person record.");
	  *errflg = true;
	  return nullPValue;
	}
    }
  else
    {
      scriptError (pnode, "Argument to choosechild must be either a family or a person record.");
      *errflg = true;
      return nullPValue;
    }

  Sequence *seq;
  if (type == GRPerson)
    seq = personToChildren (gnv.value.uGNode, database->recordIndex);
  else
    seq = familyToChildren (gnv.value.uGNode, database->recordIndex);

  if (! seq || (lengthSequence (seq) < 1))
    return nullPValue;		/* NOT an error -- person/family might have no children */
  GNode *indi = chooseFromSequence (seq, DOASK1, _(qSifonei), _(qSnotonei), chooseTypeDefault);
  if (! indi)
    return nullPValue;		/* user cancelled */

  return PVALUE(PVPerson, uGNode, indi);
}


/* __chooseindi -- have user choose person from a set
   usage: chooseindi (SET) --> INDI.  */

PValue __chooseindi (PNode *pnode, Context *context, bool* errflg)
{
  PValue svalue = evaluate (pnode->arguments, context, errflg);
  if (*errflg || (svalue.type != PVSequence))
    {
      scriptError (pnode, "Argument to chooseindi must be a set.");
      *errflg = true;
      return nullPValue;
    }
  Sequence *seq = svalue.value.uSequence;
  if (! seq || (lengthSequence (seq) < 1))
    {
      scriptError (pnode, "Argument to chooseindi must be a non-empty set.");
      *errflg = true;
      return nullPValue;
    }
  GNode *indi = chooseFromSequence (seq, DOASK1, _(qSifonei), _(qSnotonei), chooseTypeDefault);
  if (! indi)
    return nullPValue;

  return PVALUE(PVPerson, uGNode, indi);
}

/* __choosespouse -- have user choose spouse of person.
   usage: choosespouse(INDI|FAM) --> INDI.  */

PValue __choosespouse (PNode *pnode, Context *context, bool* errflg)
{
  GNode *fam = 0;
  GNode *indi = 0;
  Database *database = context->database; /* XXX make database an optional argument XXX */

  PValue pvalue = evaluate (pnode->arguments, context, errflg);
  if (*errflg)
    {
      scriptError (pnode, "Error evaluating argument to choosespouse.");
      return nullPValue;
    }
  switch (pvalue.type)
    {
    case GRPerson:
      indi = pvalue.value.uGNode;
      break;
    case GRFamily:
      fam = pvalue.value.uGNode;
      break;
    default:
      scriptError (pnode, "Argument to choosespouse must be either a person or a family.");
      *errflg = true;
      return nullPValue;
    }
  if (! indi && ! fam)
    {
      /* null GNode* */
      scriptError (pnode, "Argument to choosespouse must be a non-null person or family.");
      *errflg = true;
      return nullPValue;
    }
  Sequence *seq;

  if (indi)
    seq = personToSpouses (indi, database->recordIndex);
  else
    seq = familyToSpouses (fam, database);

  if (lengthSequence (seq) < 1)
    return nullPValue;		/* no spouses */

  GNode *newindi = chooseFromSequence (seq, DOASK1, _(qSifonei), _(qSnotonei), chooseTypeDefault);
  if (newindi)
    return PVALUE (PVPerson, uGNode, newindi);
  else
    return nullPValue;
}

/* __choosesubset -- have user choose subset from a set.
   usage: choosesubset (SET) --> SET.  */

PValue __choosesubset (PNode *pnode, Context *context, bool* errflg)
{
  PValue svalue = evaluate (pnode->arguments, context, errflg);
  if (*errflg || (svalue.type != PVSequence))
    {
      scriptError (pnode, "Argument to choosesubset must be a set.");
      *errflg = true;
      return nullPValue;
    }
  Sequence *seq = svalue.value.uSequence;
  if (! seq || (lengthSequence (seq) < 1))
    {
      scriptError (pnode, "Argument to choosesubset must be a non-empty set.");
      *errflg = true;
      return nullPValue;
    }
  Sequence *newseq = copySequence (seq);
  int retval = chooseListFromSequence (_(qSnotonei), newseq, chooseTypeDefault);
  if (retval == -1)
    return nullPValue;

  return PVALUE(PVSequence, uSequence, newseq);
}

/* __choosefam -- have user choose family of person.
   usage: choosefam (INDI) --> FAM.  */

PValue __choosefam (PNode *pnode, Context *context, bool* errflg)
{
  PValue indi = evaluate (pnode->arguments, context, errflg);
  Database *database = context->database;

  if (*errflg)
    {
      scriptError (pnode, "Error evaluating argument to choosefam.");
      return nullPValue;
    }
  if (indi.type != PVPerson)
    {
      scriptError (pnode, "Argument to choosefam must be a person.");
      *errflg = true;
      return nullPValue;
    }

  Sequence *seq = personToFamilies (indi.value.uGNode, false, database->recordIndex);

  if (! seq)
    return nullPValue;		/* not an error, indi is not a spouse in any family */
  GNode *fam = chooseFromSequence (seq, DOASK1, _(qSifonei), _(qSnotonei), chooseTypeDefault);
  if (fam)
    return PVALUE (PVFamily, uGNode, fam);
  else
    return nullPValue;
}

/* __getindiset -- have user identify set of persons
   usage: getindiset (IDEN, [,STRING]) --> VOID */

PValue __getindiset (PNode *pnode, Context *context, bool* errflg)
{
  PNode *iden = pnode->arguments;
  if (iden->type != PNIdent)
    {
      *errflg = true;
      scriptError (pnode, "First argument to getindiset must be an identifier.");
      return nullPValue;
    }
  String msg = _("Identify list of persons for program:");
  if (iden->next)
    {
      msg = evaluateString (iden->next, context, errflg);
      if (*errflg || (! msg) || (*msg == 0))
	{
	  scriptError (pnode, "The second argument to getindiset, if supplied, must be a non-empty string.");
	  *errflg = true;
	  return nullPValue;
	}
    }
  Sequence *seq = rptui_ask_for_indi_list (msg, true, currentDatabase);
  if (seq)
    nameSortSequence (seq);
  assignValueToSymbol (context, iden->identifier,
		       PVALUE (PVSequence, uSequence, seq));
  return nullPValue;
}

/* __genindiset -- identify set of persons that 'match' a string
   usage: genindiset (STRING) --> SET.  */
PValue __genindiset (PNode *pnode, Context *context, bool *errflg)
{
  CString string = evaluateString (pnode->arguments, context, errflg);
  if (*errflg || (! string) || (*string == 0))
    {
      scriptError (pnode, "The argument to genindiset must be a non-empty string.");
      *errflg = true;
      return nullPValue;
    }
  Sequence *seq = stringToSequence (string, context->database);
  if (! seq || (lengthSequence (seq) <= 0))
    return nullPValue;		/* not an error -- no matches */

  return PVALUE (PVSequence, uSequence, seq);
}

/* getint (variable [,string]) --> void

   prompts for an integer using 'string'.  If omitted, uses a compiled
   in prompt.  Assigns the returned value to 'variable'.  Returns
   void.  */

PValue __getint (PNode *pnode, Context *context, bool* eflg)
{
  PNode *iden = pnode->arguments;
  if (iden->type != PNIdent)
    {
      *eflg = true;
      scriptError (pnode, "The first argument to getint must be a variable");
      return nullPValue;
    }
  CString prompt = _("Enter integer for program");
  if (iden->next)
    {
      PValue pvalue = evaluate (iden->next, context, eflg);
      if (*eflg || (pvalue.type != PVString) || ! pvalue.value.uString)
	{
	  *eflg = true;
	  scriptError (pnode, "the second argument to getint, if supplied, must be a string");
	  return nullPValue;
	}
      prompt = pvalue.value.uString;
    }
  int num;
  if (! rptui_ask_for_int (prompt, &num))
    {
      *eflg = true;
      /* should we call scriptError?  LL is silent, no message */
      return nullPValue;
    }
  assignValueToSymbol (context, iden->identifier, PVALUE (PVInt, uInt, (long)num));
  return nullPValue;
}

PValue __getstr (PNode *pnode, Context *context, bool* eflg)
{
  char buffer[PATH_MAX];
  PNode *iden = pnode->arguments;
  if (iden->type != PNIdent)
    {
      *eflg = true;
      scriptError (pnode, "The first argument to getstr must be a variable");
      return nullPValue;
    }
  CString prompt = _(qSchoostrttl);

  if (iden->next)
    {
      PValue pvalue = evaluate (iden->next, context, eflg);
      if (*eflg || (pvalue.type != PVString) || ! pvalue.value.uString)
	{
	  *eflg = true;
	  scriptError (pnode, "the second argument to getstr, if supplied, must be a string");
	  return nullPValue;
	}
      prompt = pvalue.value.uString;
    }
  if (! ask_for_string (prompt, _(qSaskstr), buffer, sizeof (buffer)))
    {
      *eflg = true;
      //buffer[0] = 0;
      /* should we call scriptError?  LL is silent, no message */
      return nullPValue;
    }
  assignValueToSymbol (context, iden->identifier, createStringPValue (buffer));
  return nullPValue;
}

#if 0				/* renamed -- Tom has now implemented __index */
/* index(str, sub, occurrence) --> int

   returns the 1-based index of the 'occurrence'th instance of sub
   within str; 0 means not found.  */

PValue __dctindex (PNode *pnode, Context *context, bool* errflg)
{
  PNode *args = pnode->arguments;
  CString str = evaluateString (args, context, errflg);
  if (*errflg || (! str) || (*str == 0))
    {
      scriptError (pnode, "The first argument to index must be a string.");
      *errflg = true;
      return nullPValue;
    }
  args = args->next;
  CString sub = evaluateString (args, context, errflg);
  if (*errflg || (! sub) || (*sub == 0))
    {
      scriptError (pnode, "The second argument to index must be a string.");
      *errflg = true;
      return nullPValue;
    }
  args = args->next;
  int occurrence = evaluateInteger (args, context, errflg);
  if (*errflg || (! occurrence) || (occurrence <= 0))
    {
      scriptError (pnode, "The third argument to index must be a positive integer.");
      *errflg = true;
      return nullPValue;
    }
  int location = ll_index (str, sub, occurrence);

  return PVALUE (PVInt, uInt, (long)location);
}

/* index -- Find nth occurrence of sub in str (uses KMP)
   STRING str:  the text being searched
   STRING sub:  the substring being sought
   INT num:     which occurrence we want
   return value is 1-based index (or 0 if not found)  */

static int
ll_index (CString str, CString sub, int num)
{
	int result=0;
	String pi=0;
	if (!str || !sub || *str == 0 || *sub == 0) return 0;
	pi = stdalloc(strlen(sub)+1);
	compute_pi(pi, sub);
	result = kmp_search(pi, str, sub, num);
	strfree(&pi);
	return result;
}
/* kmp_search -- Perform KMP search for substring
   STRING pi:   the KMP index to avoid backtracking
   STRING str:  the text being searched
   STRING sub:  the substring being sought
   INT num:     which occurrence we want
   return value is 1-based index (or 0 if not found)  */

static int
kmp_search (CString pi, CString str, CString sub, int num)
{
	int i, q = 0, found = 0;
	int n = strlen(str);
	int m = strlen(sub);
	for (i = 1; i <= n; i++) {
		while (q > 0 && sub[q] != str[i-1])
			q = pi[q];
		if (sub[q] == str[i-1]) q++;
		if (q == m) {
			if (++found == num) return i - m + 1;
			q = pi[q];
		}
	}
	return 0;
}

/* compute_pi -- Support routine for index */

static void
compute_pi (String pi, CString sub)
{
	int m = strlen(sub), k = 0, q;
	pi[1] = 0;
	for (q = 2; q <= m; q++) {
		while (k > 0 && sub[k] != sub[q-1])
			k = pi[k];
		if (sub[k] == sub[q-1]) k++;
		pi[q] = k;
	}
}
#endif

/* __getproperty -- return property string.

 usage: getproperty(STRING) --> STRING.  */

PValue __getproperty (PNode *pnode, Context *context, bool *eflg)
{
  PNode *args = pnode->arguments;
  CString str = evaluateString (args, context, eflg);
  if (*eflg || (! str) || (*str == 0))
    {
      scriptError (pnode, "The argument to getproperty must be a string.");
      *eflg = true;
      return nullPValue;
    }
  str = get_property (str);
  return createStringPValue(str);
}

/* __menuchoose -- have user choose from a list of options

   usage: menuchoose (LIST [,STRING]) --> INT.  */

PValue __menuchoose (PNode *pnode, Context *context, bool *eflg)
{
  CString msg = 0;		/* prompt */
  CString ttl = _("Please choose from the following list."); /* default prompt */
  PNode *args = pnode->arguments;
  List *list = 0;

  PValue plist = evaluate (args, context, eflg);
  if (plist.type != PVList)
    {
      scriptError (pnode, "The first argument to menuchoose must be a list.");
      *eflg = true;
      return nullPValue;
    }
  list = plist.value.uList;

  if (args->next)
    {
      CString str = evaluateString (args->next, context, eflg);
      if (*eflg || (! str) || (*str == 0))
	{
	  scriptError (pnode, "The second argument to menuchoose, if supplied, must be a string.");
	  *eflg = true;
	  return nullPValue;
	}
      msg = str;
    }
  else
    msg = ttl;
  /* done with arguments, now go through the list and get the strings */
  int len = lengthList (list);
  if (len == 0)
    {
      /* XXX list is empty XXX */
    }
  String *strngs = stdalloc (len * sizeof (String));
  int ndx = 0;

  FORLIST (list, el)
    strngs[ndx] = ((PValue *)el)->value.uString;
    ndx++;
  ENDLIST
  ndx = rptui_chooseFromArray (msg, len, strngs);
  for (int ndx2 = 0; ndx2 < len; ndx2++)
    stdfree (strngs[ndx2]);
  stdfree (strngs);
  return PVALUE (PVInt, uInt, (ndx + 1));
}
