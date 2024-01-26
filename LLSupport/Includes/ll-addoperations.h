/* ..._to_dbase replacements */
extern bool AddOrUpdatePersonInDatabase (GNode *person, Database *database);
extern bool AddOrUpdateFamilyInDatabase (GNode *family, Database *database);
extern bool AddOrUpdateSourceInDatabase (GNode *source, Database *database);
extern bool AddOrUpdateEventInDatabase (GNode *event, Database *database);
extern bool AddOrUpdateOtherInDatabase (GNode *other, Database *database);

/* check standard keys, rekey, lists of unused keys, function to create lists */

/* xref functions -- xreffile.h */
