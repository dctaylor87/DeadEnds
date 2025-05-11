/*
 * @progname       st_convert.li
 * @version        1.01 (2002-12-14)
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description
 *
 * validate codeset conversion
 *
 */

include("st_convert.li")

/* entry point when invoked as st_convert.ll instead of via st_all.ll */

proc main()
{
	call testConvert()
}
