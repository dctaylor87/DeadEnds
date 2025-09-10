/*
 * @progname       st_name.ll
 * @version        1.0
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate name functions
*/

include ("st_name.li")

/* entry point when invoked as st_name.ll instead of via st_all.ll */

proc main()
{
	call testNames()
}
