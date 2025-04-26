#include <ansidecl.h>
#include <stdint.h>
#include <limits.h>		/* PATH_MAX */

#include "standard.h"
#include "refnindex.h"
#include "gnode.h"    // GNode.
#include "lineage.h"
#include "pvalue.h"   // PValue.
#include "pnode.h"    // PNode.
#include "name.h"     // getSurname, manipulateName.
#include "interp.h"
#include "recordindex.h" // searchRecordIndex.
#include "database.h"    // personIndex, familyIndex.
#include "hashtable.h"
#include "evaluate.h"  // evaluate.
#include "path.h"      // fopenPath.
#include "symboltable.h"
#include "date.h"
#include "place.h"
#include "builtintable.h"
#include "ask.h"
#include "rptui.h"
#include "denls.h"
#include "locales.h"
#include "lloptions.h"
#include "messages.h"

static void compute_pi(String pi, CString sub);
static int ll_index(CString str, CString sub, int num);
static int kmp_search(CString pi, CString str, CString sub, int num);

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
    
    String key = rptui_ask_for_indi_key(ttl, DOASK1);
    if (!key)
      return nullPValue;

    assignValueToSymbol(context->symbolTable, iden->identifier,
			PVALUE (PVString, uString, key));
    return nullPValue;
}

PValue __choosechild (PNode *pnode, Context *context, bool* eflg)
{
}

PValue __chooseindi (PNode *pnode, Context *context, bool* eflg)
{
}

PValue __choosespouse (PNode *pnode, Context *context, bool* eflg)
{
}

PValue __choosesubset (PNode *pnode, Context *context, bool* eflg)
{
}

PValue __choosefam (PNode *pnode, Context *context, bool* eflg)
{
}

PValue __getindiset (PNode *pnode, Context *context, bool* eflg)
{
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
  assignValueToSymbol (context->symbolTable, iden->identifier,
		       PVALUE (PVInt, uInt, (long)num));
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
  int num;
  if (! ask_for_string (prompt, _(qSaskstr), buffer, sizeof (buffer)))
    {
      *eflg = true;
      //buffer[0] = 0;
      /* should we call scriptError?  LL is silent, no message */
      return nullPValue;
    }
  assignValueToSymbol (context->symbolTable, iden->identifier,
		       PVALUE (PVInt, uInt, (long)num));
  return nullPValue;
}

/* index(str, sub, occurrence) --> int

   returns the 1-based index of the 'occurrence'th instance of sub
   within str; 0 means not found.  */

PValue __index (PNode *pnode, Context *context, bool* errflg)
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
  return PVALUE (PVString, uString, str);
}

#if 0
static void
makestring (PValue val, String str, INT len, bool *eflg)
{
  str[0]=0;

  switch(val.type) {
  case PVNull:
    destrapps(str, len, uu8, "<NULL>");
    break;
  case PVInt:
  case PVFloat:
    destrappf(str, len, uu8, "%f", pvalue_to_float(val));
    break;
  case PVBool:
    /* TODO: Should we localize this ? */
    destrapps(str, len, uu8, val.value.uBool ? "True" : "False");
    break;
  case PVString:
    destrapps(str, len, uu8, val.value.uString);
    break;
  case PVGNode:
    {
      /* TODO: report codeset conversion */
      GNode *node = val.value.uGNode;
      if (ntag(node)) {
	destrappf(str, len, uu8, "%s: ", ntag(node));
      }
      if (nval(node))
	destrapps(str, len, uu8, nval(node));
    }
    break;
  case PVPerson:
  case PVFamily:
  case PVSource:
  case PVEvent:
  case PVOther:
    {
      GNode *node = val.value.uGNode;
      String txt = generic_to_list_string(node, NULL, len, " ", NULL, TRUE);
      destrapps(str, len, uu8, txt);
    }
    break;
  case PVList:
    destrapps(str, len, uu8, "<List>");
    break;
  case PVTable:
    destrapps(str, len, uu8, "<Table>");
    break;
  case PVSequence:
    destrapps(str, len, uu8, "<Sequence>");
    break;
  default:
    *eflg = TRUE;
  }
}
#endif

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
#if 0
    PValue vel = (PValue) el;
    strngs[ndx] = pvalueToString (vel, false);
#else
    strngs[ndx] = (String) el;	/* XXX is it a string? XXX */
#endif
    ndx++;
  ENDLIST
  ndx = rptui_chooseFromArray (msg, len, strngs);
  for (int ndx2 = 0; ndx2 < len; ndx2++)
    stdfree (strngs[ndx2]);
  stdfree (strngs);
  return PVALUE (PVInt, uInt, ndx);
}
