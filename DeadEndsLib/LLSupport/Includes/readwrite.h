/* readwrite.h -- These are all read/write related.  In LifeLines
   these are found in hdrs/standard.h */

/* integer printf format definitions */

#define FMT_INTPTR	"%" PRIdPTR
#define FMT_INT16	"%" PRId16
#define FMT_INT16_HEX	"0x%04" PRIx16
#define FMT_INT32	"%" PRId32
#define FMT_INT32_HEX	"0x%08" PRIx32
#define FMT_INT32_HEX_06 "0x%06" PRIx32
#define FMT_INT64	"%" PRId64
#define FMT_INT64_HEX	"0x%016" PRIx64

#define FMT_INT_2	"%2d"	/* ??? */
#define FMT_INT_6	"%6d"	/* ??? */
#define FMT_INT_04	"%04d"
#define FMT_INT_LEN	22	/* sign + 20 digits + NULL */

#define FMT_INT_02	"%02d"
#define FMT_INT_03	"%03d"

#define FMT_INT		"%d"
#define SCN_INT		"%d"

extern String editfile;
extern String editstr;

/* valid.c */
extern bool pointer_value(String);
extern bool valid_indi_tree(GNode *, String*, GNode *, Database *database);
extern bool valid_fam_tree(GNode *, String*, GNode *, Database *database);
extern bool valid_name(String);
extern bool valid_node_type(GNode *node, char ntype, String *pmsg,
			    GNode *node0, Database *database);
extern bool valid_sour_tree(GNode *, String*, GNode *, Database *database);
extern bool valid_even_tree(GNode *, String*, GNode *, Database *database);
extern bool valid_othr_tree(GNode *, String*, GNode *, Database *database);

/* write.c */

/* XXX interface and/or name might change XXX */
extern GNode *file_to_node (String fname, XLAT ttmi, String *pmsg, bool *pemp);
