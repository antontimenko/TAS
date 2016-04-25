#ifndef _TSRAWSENTENCE_H_
#define _TSRAWSENTENCE_H_

#include "TSGlobal.h"
#include "TSInteger.h"
#include "TSCodePosition.h"
#include "TSInstruction.h"
#include "TSPseudoSentence.h"
#include "TSUniquePtr.h"
#include "TSSentence.h"

struct TSRawNumber {
    TSInteger num;
    optional<TSLabel> label;
    bool isNotFinal;
};

class TSSentenceSize;

class TSRawSentence {
public:
    virtual tuple<string, vector<string>> present(const map<string, TSLabel> &labelMap) const = 0;
    
    inline TSRawSentence(TSCodePosition pos, TSAssume assume) :
        _pos(pos),
        _assume(assume)
    {}

    inline TSCodePosition pos() const {
        return _pos;
    }

    inline TSAssume assume() const {
        return _assume;
    }
private:
    TSCodePosition _pos;
    TSAssume _assume;
};

class TSRawInstructionSentence : public TSRawSentence {
public:
    typedef TSInstructionSentence::SegmentPrefix SegmentPrefix;

    class Operand {
    public:
        string present(const map<string, TSLabel> &labelMap) const;
        TSOperandMask::Mask mask;
        TSRawNumber rawNum;
    };

    typedef TSInstruction::Instruction Instruction;
    typedef tuple<Operand, TSCodePosition> OperandContainer;

    TSRawInstructionSentence(const TSPseudoSentence &pseudoSentence, const map<string, TSLabel> &labelMap);
    virtual tuple<string, vector<string>> present(const map<string, TSLabel> &labelMap) const override;
private:
    optional<SegmentPrefix> segmentPrefix;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;

    friend shared_ptr<TSSentence> constructSentenceFromRaw(const TSRawSentence &rawSentence, vector<bool> linkVector);
    friend vector<TSSentencesSegment> constructSentences(const vector<TSRawSentencesSegment> &rawSentencesSegmentContainer);
};

class TSRawDataSentence : public TSRawSentence {
public:
    typedef TSRawNumber Operand;
    typedef tuple<Operand, TSCodePosition> OperandContainer;
    typedef TSInstruction::DataIdentifier DataIdentifier;

    TSRawDataSentence(const TSPseudoSentence &pseudoSentence, const map<string, TSLabel> &labelMap);
    virtual tuple<string, vector<string>> present(const map<string, TSLabel> &labelMap) const override;
private:
    DataIdentifier dataIdentifier;
    vector<OperandContainer> operandContainerVector;

    friend shared_ptr<TSSentence> constructSentenceFromRaw(const TSRawSentence &rawSentence, vector<bool> linkVector);
    friend vector<TSSentencesSegment> constructSentences(const vector<TSRawSentencesSegment> &rawSentencesSegmentContainer);
};

struct TSRawSentencesSegment {
    string segName;
    vector<shared_ptr<TSRawSentence>> rawSentences;
};

TSRawInstructionSentence::SegmentPrefix getSegmentOverridePrefix(TSToken::Register reg);

vector<TSRawSentencesSegment> constructRawSentences(const vector<TSPseudoSentencesSegment> &pseudoSentencesSegmentContainerVector,
                                                    const map<string, TSLabel> &labelMap);

#endif
