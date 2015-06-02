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

struct TSLabel
{
    optional<TSToken::DataIdentifier> dataIdentifier;
    size_t ptr;
    string segName;
};

typedef tuple<vector<TSPseudoSentencesSegmentContainer>, map<string, TSLabel>> TSPseudoSentenceSplitType;

TSPseudoSentenceSplitType splitPseudoSentences(const vector<TSTokenContainersSegmentContainer> &segmentTokenContainerVector);

#endif
