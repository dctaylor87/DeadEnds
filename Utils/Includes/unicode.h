//
//  unicode.h
//  JustParsing
//
//  Created by Thomas Wetmore on 6/20/23.
//

#ifndef unicode_h
#define unicode_h

#include <stdio.h>

extern bool isCombiningCharacter(unsigned int codePoint);
extern char* normalizeDType(const char* input);


#endif /* unicode_h */
