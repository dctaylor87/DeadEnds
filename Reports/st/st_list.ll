/*
 * @progname       st_list.li
 * @version        1.16 (2007-05-06)
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate list functions
*/

include ("st_list.li")

/* entry point when invoked as st_list.ll instead of via st_all.ll */

proc main()
{
	call testLists()
}
