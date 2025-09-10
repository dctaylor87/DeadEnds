/*
 * @progname       st_trig.ll
 * @version        0.9 (2008-01-05)
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate trig functions
*/

include ("st_trig.li")

/* entry point when invoked as st_trip.ll instead of via st_all.ll */

proc main()
{
	call testTrig()
}
