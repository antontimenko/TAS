#ifndef _TSDIAGNOSTICS_H_
#define _TSDIAGNOSTICS_H_

#include "TSGlobal.h"
#include "TSInteger.h"
#include "TSToken.h"
#include "TSLexeme.h"
#include "TSPseudoSentence.h"
#include "TSRawSentence.h"
#include "TSSentence.h"

namespace TSColor
{
constexpr auto Reset = "\033[0m";
constexpr auto Black = "\033[30m";
constexpr auto Red = "\033[31m";
constexpr auto Green = "\033[32m";
constexpr auto Yellow = "\033[33m";
constexpr auto Blue = "\033[34m";
constexpr auto Magenta = "\033[35m";
constexpr auto Cyan = "\033[36m";
constexpr auto White = "\033[37m";
constexpr auto BBlack = "\033[1m\033[30m";
constexpr auto BRed = "\033[1m\033[31m";
constexpr auto BGreen = "\033[1m\033[32m";
constexpr auto BYellow = "\033[1m\033[33m";
constexpr auto BBlue = "\033[1m\033[34m";
constexpr auto BMagenta = "\033[1m\033[35m";
constexpr auto BCyan = "\033[1m\033[36m";
constexpr auto BWhite = "\033[1m\033[37m";
}

void printError(string text);
void printCompileError(string text, const string &sourceFileContents, TSCodePosition pos);
void printTokenTable(const vector<TSTokenContainer> &tokenContainerVector);
void printTokenTable(const vector<TSTokenContainer> &tokenContainerVector, const vector<TSLexemeContainer> &lexemeContainerVector);
void printEquTable(const map<string, TSInteger> &equMap);
void printPseudoLabelTable(const map<string, TSLabelParamType> &labelMap);
void printPseudoSentenceTable(const vector<TSPseudoSentencesSegmentContainer> &segmentPseudoSentenceVector);
void printRawSentenceTable(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainerVector);
void printSentenceTable(const vector<TSSentencesSegmentContainer> &sentencesSegmentContainerVector);

template<typename T, typename U>
typename map<T, U>::const_iterator findByValue(const map<T, U> &source, U value)
{
    for (auto it = source.begin(); it != source.end(); ++it)
    {
        if (value == it->second)
            return it;
    }

    return source.end();
}

#endif
