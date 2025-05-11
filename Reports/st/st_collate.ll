/*
 * @progname       st_collate.ll
 * @version        1.0
 * @author         Perry Rapp
 * @category       self-test
 * @output         none
 * @description
 *
 * validate collation
 *
 */

include ("st_collate.li")

/* entry point when invoked as st_collate.ll instead of via st_all.ll */

proc main()
{
	call testCollate()
}
