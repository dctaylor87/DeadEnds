/*
 * @progname       st_db.li
 * @version        1.26 [of 2005-02-01]
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description
 *
 * Exercise some database functions.
 * Dumps some of each type of record, followed by all 3 gengedcoms.
 *
 */

include ("st_db.li")

/* entry point when invoked as st_db.ll instead of via st_all.ll */

proc main()
{
	call exerciseDb()
}
