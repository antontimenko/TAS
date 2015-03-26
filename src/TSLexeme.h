#ifndef _TSLEXEME_H_
#define _TSLEXEME_H_

#include "TSGlobal.h"

struct TSLexemeContainer
{
    const size_t row;
    const size_t column;
    const string lexeme;
};

vector<TSLexemeContainer> constructLexemeContainerVector(const string &sourceFileContents);
vector<TSLexemeContainer> convertLexemeContainerVectorToUpperCase(vector<TSLexemeContainer> &lexemeContainerVector);

#endif
