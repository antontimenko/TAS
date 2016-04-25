#ifndef _LEXEME_H_
#define _LEXEME_H_

#include "Global.h"

extern const char cLF;
extern const char cCR;

bool isCharIdentifierCompatible(char ch);
bool isCharNumberCompatible(char ch);
bool isCharSingleCharacterLexemeCompatible(char ch);
bool isCharLexemeDistributorCompatible(char ch);
bool isCharQuoteCompatible(char ch);

struct LexemeContainer {
    const size_t row;
    const size_t column;
    const string lexeme;
};

vector<LexemeContainer> constructLexemeContainerVector(const string &sourceFileContents);
vector<LexemeContainer> convertLexemeContainerVectorToUpperCase(vector<LexemeContainer> &lexemeContainerVector);

#endif
