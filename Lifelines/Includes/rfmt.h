/* reformating functions - format date or place for display callbacks
   to GUI client to tailor display as desired either or both may be
   null, meaning use date or place exactly as occurs in data */

struct tag_rfmt {
  STRING (*rfmt_date)(STRING); /* returns static buffer */
  STRING (*rfmt_plac)(STRING); /* returns static buffer */
  STRING combopic; /* stdalloc'd buffer, eg, "%1, %2" */
};

typedef struct tag_rfmt *RFMT;

extern struct tag_rfmt disp_long_rfmt;
extern struct tag_rfmt disp_shrt_rfmt;
