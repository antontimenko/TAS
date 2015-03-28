#ifndef _TSLEXEME_H_
#define _TSLEXEME_H_

#include "TSGlobal.h"

extern const char cLF;
extern const char cCR;

bool isCharIdentifierCompatible(char ch);
bool isCharNumberCompatible(char ch);
bool isCharSingleCharacterLexemCompatible(char ch);
bool isCharLexemeDistributorCompatible(char ch);
bool isCharQuoteCompatible(char ch);

struct TSLexemeContainer
{
    const size_t row;
    const size_t column;
    const string lexeme;
};

vector<TSLexemeContainer> constructLexemeContainerVector(const string &sourceFileContents);
vector<TSLexemeContainer> convertLexemeContainerVectorToUpperCase(vector<TSLexemeContainer> &lexemeContainerVector);

#endif
