/* addops.c */

extern bool addChildToFamily (GNode *child, GNode *family, int index,
			      Database *database);
extern bool addSpouseToFamily (GNode *spouse, GNode *family, SexType sext,
			       Database *database);

/* removeops.c */

extern bool removeChildFromFamily (GNode *child, GNode *family,
				   Database *database);
extern bool removeSpouseFromFamily (GNode *spouse, GNode *family, Error *error);
