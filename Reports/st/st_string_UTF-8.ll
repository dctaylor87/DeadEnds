/*
 * @progname       st_string_UTF-8.li
 * @version        1.1
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description    validate string functions on UTF-8
*/

include ("st_string_UTF-8.li")

/* entry point when invoked as str_string_UTF-8.ll instead of via st_all.ll */

proc main()
{
	call testStrings_UTF_8()
}
