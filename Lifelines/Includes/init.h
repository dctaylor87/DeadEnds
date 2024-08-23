extern void close_lifelines (void);
extern bool init_lifelines_global (String configfile, String * pmsg,
				      void (*notify)(String db, bool opening));
extern bool init_lifelines_postdb (void);
extern void update_useropts (ATTRIBUTE_UNUSED void *uparm);

extern String environ_determine_tempfile (void);
extern String environ_determine_editor (int program);
