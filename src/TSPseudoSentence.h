#ifndef _TSPSEUDOSENTENCE_H_
#define _TSPSEUDOSENTENCE_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSPreprocessor.h"
#include "TSMath.h"
#include "TSInstruction.h"

struct TSPseudoSentence
{
    TSTokenContainer baseTokenContainer;
    vector<vector<TSTokenContainer>> operandVector;
};

struct TSSegmentPseudoSentence
{
    string name;
    vector<TSPseudoSentence> pseudoSentenceVector;
};

enum class TSLabelType
{
    LABEL,
    DATA
};

typedef tuple<TSLabelType, TSToken::DataIdentifier, size_t> TSLabelParamType;
typedef tuple<vector<TSSegmentPseudoSentence>, map<string, TSLabelParamType>> TSPseudoSentenceSplitType;

TSPseudoSentenceSplitType splitPseudoSentences(const vector<TSSegmentTokenContainer> &segmentTokenContainerVector);

#endif
