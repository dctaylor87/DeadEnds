/* ..._to_dbase replacements */
extern bool addOrUpdatePersonInDatabase (GNode *person, Database *database);
extern bool addOrUpdateFamilyInDatabase (GNode *family, Database *database);
extern bool addOrUpdateSourceInDatabase (GNode *source, Database *database);
extern bool addOrUpdateEventInDatabase (GNode *event, Database *database);
extern bool addOrUpdateOtherInDatabase (GNode *other, Database *database);

/* check standard keys, rekey, lists of unused keys, function to create lists */

/* xref functions -- xreffile.h */
