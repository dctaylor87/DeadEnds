
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"
#include "denls.h"
#include "sequence.h"
#include "ui.h"
#include "messages.h"
#include "feedback.h"
#include "de-strings.h"

/* ask_for_char -- Ask user for character
   ttl:   [IN]  1nd line displayed
   prmpt: [IN]  2nd line text before cursor
   ptrn:  [IN]  List of allowable character responses   */

int
ask_for_char (CString ttl, CString prmpt, CString ptrn)
{
	return ask_for_char_msg(NULL, ttl, prmpt, ptrn);
}

/* ask_yes_or_no -- Ask yes or no question
   ttl:  [IN]  title to display  */

bool
ask_yes_or_no (CString ttl)
{
	int c = ask_for_char(ttl, _(qSaskynq), _(qSaskynyn));
	return yes_no_value(c);
}

/* ask_yes_or_no_msg -- Ask yes or no question with message
   msg:   [IN]  top line displayed
   ttl:   [IN]  2nd line displayed  */

bool
ask_yes_or_no_msg (CString msg, CString ttl)
{
	int c = ask_for_char_msg(msg, ttl, _(qSaskynq), _(qSaskynyn));
	return yes_no_value(c);
}

/* ask_for_db_filename -- Ask user for DeadEnds database directory
   ttl:   [IN]  title of question (1rst line)
   prmpt: [IN]  prompt of question (2nd line)  */

bool
ask_for_db_filename (CString ttl, CString prmpt, ATTRIBUTE_UNUSED CString basedir, String buffer, int buflen)
{
	return ask_for_string(ttl, prmpt, buffer, buflen);
}

/* ask_for_output_filename -- Ask user for filename to which to write
   returns static buffer
   ttl1:    [IN]  title of question (1rst line)
   prmpt:   [IN]  prompt of question (3rd line)
   buffer:  [OUT] response
   buflen:  [IN]  max size of response  */

bool
ask_for_output_filename (CString ttl, CString path, CString prmpt, String buffer, int buflen)
{
	/* curses version doesn't differentiate input from output prompts */
	return ask_for_filename_impl(ttl, path, prmpt, buffer, buflen);
}

/* ask_for_input_filename -- Ask user for filename from which to read
   returns static buffer
   ttl1:    [IN]  title of question (1rst line)
   prmpt:   [IN]  prompt of question (3rd line)
   buffer:  [OUT] response
   buflen:  [IN]  max size of response  */

bool
ask_for_input_filename (CString ttl, CString path, CString prmpt, String buffer, int buflen)
{
	/* curses version doesn't differentiate input from output prompts */
	return ask_for_filename_impl(ttl, path, prmpt, buffer, buflen);
}

int
chooseFromList (CString ttl, List *list)
{
	String * array=0;
	String choice=0;
	int i=0, rtn=-1;
	int len = lengthList(list);

	if (len < 1) return -1;
	if (!ttl) ttl=_(qSdefttl);

	array = (String *) stdalloc(len*sizeof(String));
	i = 0;
	FORLIST(list, el)
	  choice = (String)el;
	ASSERT(choice);
	array[i] = strsave(choice);
	++i;
	ENDLIST
	rtn = chooseFromArray(ttl, len, array);

	for (i=0; i<len; ++i)
		strfree(&array[i]);
	stdfree(array);
	return rtn;
}

/* chooseListFromSequence -- User chooses subsequence from
   person sequence
   returns input sequence, but may have deleted elements
   called by both reports & interactive use
   ttl:  [IN]  title/caption for choice list
   seq:  [I/O] list from which to choose (user may delete items)
   returns index of where user choose select (or -1 if quit)  */

int
chooseListFromSequence (CString ttl, Sequence *seq)
{
	return chooseOneOrListFromSequence(ttl, seq, true);
}

/* chooseOneFromSequence --
   Choose a single person from indiseq
   Returns index of selected item (or -1 if user quit)
   ttl:  [IN]  title  */
 

int
chooseOneFromSequence (CString ttl, Sequence *seq)
{
	return chooseOneOrListFromSequence(ttl, seq, false);
}

/* yes_no_value -- Convert character to true if y(es)  */

bool
yes_no_value (int c)
{
	String ptr;
	for (ptr = _(qSaskyY); *ptr; ptr++) {
		if (c == *ptr) return true;
	}
	return false;
}
