//
//  DeadEnds
//
//  validate.h
//
//  Created by Thomas Wetmore on 12 April 2023.
//  Last changed on 18 December 2023.
//

#ifndef validate_h
#define validate_h

#include "database.h"
#include "errors.h"

extern bool validateDatabase(Database*, ErrorLog*);
extern bool validatePersonIndex(Database*, ErrorLog*);
extern bool validateFamilyIndex(Database*, ErrorLog*);
extern int personLineNumber(GNode*, Database*);
extern bool validateIndex(RecordIndex *index);

#endif // validate_h
