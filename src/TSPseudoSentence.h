#ifndef _TSPSEUDOSENTENCE_H_
#define _TSPSEUDOSENTENCE_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSPreprocessor.h"
#include "TSMath.h"
#include "TSInstruction.h"
#include "TSOperandMask.h"

class TSAssume {
public:
    inline void setSegment(string segName, TSOperandMask::Mask segReg) {
        assumeMap[segName] = segReg;
    }
    
    inline map<string, TSOperandMask::Mask> getMap() const {
        return assumeMap;
    }
private:
    map<string, TSOperandMask::Mask> assumeMap;
};

struct TSPseudoSentence {
    TSTokenContainer baseTokenContainer;
    vector<vector<TSTokenContainer>> operandsTokenContainerVector;
    TSAssume assume;
};

struct TSPseudoSentencesSegment {
    string segName;
    vector<TSPseudoSentence> pseudoSentences;
};

class TSLabel {
public:
    optional<TSToken::DataIdentifier> dataIdentifier;
    size_t ptr;
    string segName;

    inline bool operator==(const TSLabel &label) const {
        return (dataIdentifier == label.dataIdentifier) &&
               (ptr == label.ptr) &&
               (segName == label.segName);
    }
};

class TSLabelFinder {
public:
    inline TSLabelFinder(TSLabel label) :
        label(label)
    {}

    const TSLabel label;

    inline bool operator()(const pair<string, TSLabel> &labelPair) const {
        return label == labelPair.second;
    }
};

tuple<vector<TSPseudoSentencesSegment>, map<string, TSLabel>> splitPseudoSentences(const vector<TSTokenSegment> &segmentTokenContainerVector);

#endif
