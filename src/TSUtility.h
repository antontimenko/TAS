#ifndef _TSUTILITY_H_
#define _TSUTILITY_H_

#include "TSGlobal.h"

extern const char cLF;
extern const char cCR;

bool isCharIdentifierCompatible(char ch);
bool isCharNumberCompatible(char ch);
bool isCharSingleCharacterLexemCompatible(char ch);
bool isCharLexemDistributorCompatible(char ch);
bool isCharCommentCompatible(char ch);
bool isCharQuoteCompatible(char ch);

#endif
