/* looking at the Lifelines code, a keynum is sometimes an int (32
   bits), sometimes an int64_t (64 bits, obviously), and sometimes an
   INT (which is 32 bits [x86] or 64 bits [x86_64] -- depending on
   platform) */

typedef int64_t		KEYNUM_TYPE;

#define MAXKEYNUMBER	999999999999L
/* XXX not sure if MAXKEYWIDTH is still used.  Have eliminated most uses.  XXX */
/* was 13, changed to 26 in case we switch from uint32_t to uint64_t */
#define MAXKEYWIDTH	26	/* does NOT include NUL nor surrounding @'s */

#define PRIdKEYNUM	PRId64
#define FMT_KEYNUM	"%" PRIdKEYNUM
