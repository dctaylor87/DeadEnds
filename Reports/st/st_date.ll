/*
 * @progname       st_date.li
 * @version        1.42 (2007-12-24)
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description
 *
 * validate date functions
 *
 */

include("st_date.li")

/* entry point when invoked as st_date.ll instead of via st_all.ll */

proc main()
{
	call testDates()
}
