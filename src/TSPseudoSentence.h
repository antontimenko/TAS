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

class TSLabel
{
public:
    optional<TSToken::DataIdentifier> dataIdentifier;
    size_t ptr;
    string segName;

    inline bool operator==(const TSLabel &label) const
    {
        return (dataIdentifier == label.dataIdentifier) &&
               (ptr == label.ptr) &&
               (segName == label.segName);
    }
};

class TSLabelFinder
{
public:
    inline TSLabelFinder(TSLabel label) :
        label(label)
    {}

    const TSLabel label;

    inline bool operator()(const pair<string, TSLabel> &labelPair) const
    {
        return label == labelPair.second;
    }
};

typedef tuple<vector<TSPseudoSentencesSegmentContainer>, map<string, TSLabel>> TSPseudoSentenceSplitType;

TSPseudoSentenceSplitType splitPseudoSentences(const vector<TSTokenContainersSegmentContainer> &segmentTokenContainerVector);

#endif
