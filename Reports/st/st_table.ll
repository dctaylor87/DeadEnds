/*
 * @progname       st_table.ll
 * @version        1.0 (2005-02-01)
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate table functions
*/

include ("st_table.li")

/* entry point when invoked as st_table.ll instead of via st_all.ll */

proc main()
{
	call testTables()
}
