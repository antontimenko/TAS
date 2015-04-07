#ifndef _TSPARSER_H_
#define _TSPARSER_H_

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

enum class LabelType
{
    LABEL,
    DATA
};

void parse(const vector<TSSegmentTokenContainer> &segmentTokenContainerVector);

#endif
