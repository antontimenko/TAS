#ifndef _TSDIAGNOSTICS_H_
#define _TSDIAGNOSTICS_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSLexeme.h"

namespace TSColor
{
extern const char *Reset;
extern const char *Black;
extern const char *Red;
extern const char *Green;
extern const char *Yellow;
extern const char *Blue;
extern const char *Magenta;
extern const char *Cyan;
extern const char *White;
extern const char *BBlack;
extern const char *BRed;
extern const char *BGreen;
extern const char *BYellow;
extern const char *BBlue;
extern const char *BMagenta;
extern const char *BCyan;
extern const char *BWhite;
}

void printError(string text);
void printCompileError(string text, const string &sourceFileContents, size_t row, size_t column, size_t length);
void printLexemeTable(const vector<TSLexemeContainer> &lexemeContainerVector, const vector<TSTokenContainer> &tokenContainerVector);

#endif
