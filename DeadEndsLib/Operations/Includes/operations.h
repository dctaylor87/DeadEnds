extern bool addChildToFamily (GNode *child, GNode *family, int index, Database *database);
extern bool addSpouseToFamily (GNode* spouse, GNode* family, SexType sext, Database* database);
extern GNode* createFamily(GNode* husb, GNode* wife, GNode* chil, GNode* rest, Database* database);
