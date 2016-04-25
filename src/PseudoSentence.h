#ifndef _PSEUDOSENTENCE_H_
#define _PSEUDOSENTENCE_H_

#include "Global.h"
#include "Token.h"
#include "Preprocessor.h"
#include "Math.h"
#include "Instruction.h"
#include "OperandMask.h"

class Assume {
public:
    inline void setSegment(string segName, OperandMask::Mask segReg) {
        assumeMap[segName] = segReg;
    }
    
    inline map<string, OperandMask::Mask> getMap() const {
        return assumeMap;
    }
private:
    map<string, OperandMask::Mask> assumeMap;
};

struct PseudoSentence {
    TokenContainer baseTokenContainer;
    vector<vector<TokenContainer>> operandsTokenContainerVector;
    Assume assume;
};

struct PseudoSentencesSegment {
    string segName;
    vector<PseudoSentence> pseudoSentences;
};

class Label {
public:
    optional<Token::DataIdentifier> dataIdentifier;
    size_t ptr;
    string segName;

    inline bool operator==(const Label &label) const {
        return (dataIdentifier == label.dataIdentifier) &&
               (ptr == label.ptr) &&
               (segName == label.segName);
    }
};

class LabelFinder {
public:
    inline LabelFinder(Label label) :
        label(label)
    {}

    const Label label;

    inline bool operator()(const pair<string, Label> &labelPair) const {
        return label == labelPair.second;
    }
};

tuple<vector<PseudoSentencesSegment>, map<string, Label>> splitPseudoSentences(const vector<TokenSegment> &segmentTokenContainerVector);

#endif
