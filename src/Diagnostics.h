#ifndef _DIAGNOSTICS_H_
#define _DIAGNOSTICS_H_

#include "Global.h"
#include "Integer.h"
#include "Token.h"
#include "Lexeme.h"
#include "PseudoSentence.h"
#include "RawSentence.h"
#include "Sentence.h"

namespace Color {

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
void printCompileError(string text, const string &sourceFileContents, CodePosition pos);
void printTokenTable(const vector<TokenContainer> &tokenContainerVector);
void printTokenTable(const vector<TokenContainer> &tokenContainerVector, const vector<LexemeContainer> &lexemeContainerVector);
void printEquTable(const map<string, Integer> &equMap);
void printPseudoLabelTable(const map<string, Label> &labelMap);
void printPseudoSentenceTable(const vector<PseudoSentencesSegment> &segmentPseudoSentenceVector, bool printAssumes = false);
void printRawSentenceTable(const vector<RawSentencesSegment> &rawSentencesSegmentContainerVector, const map<string, Label> &labelMap, bool printAssumes = false);
void printSentenceTable(const vector<SentencesSegment> &sentencesSegmentContainerVector, bool printAssumes = false);
void printListing(const vector<SentencesSegment> &sentencesSegmentContainerVector);
void printListing(const vector<SentencesSegment> &sentencesSegmentContainerVector, const tuple<vector<PseudoSentencesSegment>, map<string, Label>> &pseudoSentenceSplit);

template<typename T, typename U>
typename map<T, U>::const_iterator findByValue(const map<T, U> &source, U value) {
    for (auto it = source.begin(); it != source.end(); ++it) {
        if (value == it->second)
            return it;
    }

    return source.end();
}

#endif
