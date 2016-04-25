#ifndef _TSSENTENCE_H_
#define _TSSENTENCE_H_

#include "TSGlobal.h"
#include "TSInteger.h"
#include "TSOperandMask.h"
#include "TSCodePosition.h"
#include "TSInstruction.h"
#include "TSPseudoSentence.h"

class TSSentence {
public:
    virtual vector<vector<uchar>> compute() const = 0;
    virtual tuple<string, vector<string>> present() const = 0;
    
    TSSentence(TSCodePosition pos, TSAssume assume) :
        pos(pos),
        assume(assume)
    {}

    TSCodePosition pos;
    TSAssume assume;
};

class TSInstructionSentence: public TSSentence {
public:
    enum class SegmentPrefix {
        ES,
        CS,
        SS,
        DS,
        FS,
        GS
    };

    class Operand {
    public:
        string present() const;
        TSOperandMask::Mask mask;
        TSInteger num;
        bool isLinkable;
        optional<string> segName;
    };

    typedef TSInstruction::Instruction Instruction;
    typedef tuple<Operand, TSCodePosition> OperandContainer;

    inline TSInstructionSentence(TSCodePosition pos, TSAssume assume, optional<SegmentPrefix> segmentPrefix, Instruction instruction, vector<OperandContainer> operandContainerVector) :
        TSSentence(pos, assume),
        segmentPrefix(segmentPrefix),
        instruction(instruction),
        operandContainerVector(operandContainerVector)
    {}

    virtual vector<vector<uchar>> compute() const override;
    virtual tuple<string, vector<string>> present() const override;

    optional<SegmentPrefix> segmentPrefix;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;

    static const map<SegmentPrefix, TSOperandMask::Mask> prefixToSegRegMap;
};

class TSDataSentence : public TSSentence {
public:
    typedef TSInteger Operand;
    typedef tuple<Operand, TSCodePosition> OperandContainer;
    typedef TSInstruction::DataIdentifier DataIdentifier;

    inline TSDataSentence(TSCodePosition pos, TSAssume assume, DataIdentifier dataIdentifier, vector<OperandContainer> operandContainerVector) :
        TSSentence(pos, assume),
        dataIdentifier(dataIdentifier),
        operandContainerVector(operandContainerVector)
    {}

    virtual vector<vector<uchar>> compute() const override;
    virtual tuple<string, vector<string>> present() const override;

    DataIdentifier dataIdentifier;
    vector<OperandContainer> operandContainerVector;
};

struct TSSentencesSegment {
    string segName;
    vector<shared_ptr<TSSentence>> sentences;
};

class TSRawSentence;
struct TSRawSentencesSegment;

vector<TSSentencesSegment> constructSentences(const vector<TSRawSentencesSegment> &rawSentencesSegmentContainerVector);

#endif
