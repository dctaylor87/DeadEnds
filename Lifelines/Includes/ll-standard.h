/* looking at the Lifelines code, a keynum is sometimes an int (32
   bits), sometimes an int64_t (64 bits, obviously), and sometimes an
   INT (which is 32 bits [x86] or 64 bits [x86_64] -- depending on
   platform) */

typedef int64_t		KEYNUM_TYPE;

#define MAXKEYNUMBER	999999999999L
#define MAXKEYWIDTH	13	/* does NOT include NUL nor surrounding @'s */

#define PRIdKEYNUM	PRId64
#define FMT_KEYNUM	"%" PRIdKEYNUM
