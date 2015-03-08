#ifndef _TSLEXICALANALYZE_H_
#define _TSLEXICALANALYZE_H_

#include "TSGlobal.h"

struct TSLexemeContainer
{
    const uint row;
    const uint column;
    const string lexeme;
};

vector<TSLexemeContainer> constructLexemeContainerVector(const string &sourceFileContents);

#endif
