/*
 * @progname       st_string.li
 * @version        1.2 of 2005-01-12
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate string functions
*/

include ("st_string.li")

/* entry point when invoked as st_string.ll instead of via st_all.ll */

proc main()
{
	call testStrings()
}
