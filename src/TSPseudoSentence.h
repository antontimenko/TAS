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
    vector<vector<TSTokenContainer>> operandsTokenContainerVector;
};

typedef tuple<string, vector<TSPseudoSentence>> TSPseudoSentencesSegmentContainer;

enum class TSLabelType
{
    LABEL,
    DATA
};

typedef tuple<TSLabelType, TSToken::DataIdentifier, size_t, string> TSLabelParamType;
typedef tuple<vector<TSPseudoSentencesSegmentContainer>, map<string, TSLabelParamType>> TSPseudoSentenceSplitType;

TSPseudoSentenceSplitType splitPseudoSentences(const vector<TSTokenContainersSegmentContainer> &segmentTokenContainerVector);

#endif
