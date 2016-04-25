#ifndef _SENTENCE_H_
#define _SENTENCE_H_

#include "Global.h"
#include "Integer.h"
#include "OperandMask.h"
#include "CodePosition.h"
#include "Instruction.h"
#include "PseudoSentence.h"

class Sentence {
public:
    virtual vector<vector<uchar>> compute() const = 0;
    virtual tuple<string, vector<string>> present() const = 0;
    
    Sentence(CodePosition pos, Assume assume) :
        pos(pos),
        assume(assume)
    {}

    CodePosition pos;
    Assume assume;
};

class InstructionSentence: public Sentence {
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
        OperandMask::Mask mask;
        Integer num;
        bool isLinkable;
        optional<string> segName;
    };

    typedef InstructionNS::Instruction Instruction;
    typedef tuple<Operand, CodePosition> OperandContainer;

    inline InstructionSentence(CodePosition pos, Assume assume, optional<SegmentPrefix> segmentPrefix, Instruction instruction, vector<OperandContainer> operandContainerVector) :
        Sentence(pos, assume),
        segmentPrefix(segmentPrefix),
        instruction(instruction),
        operandContainerVector(operandContainerVector)
    {}

    virtual vector<vector<uchar>> compute() const override;
    virtual tuple<string, vector<string>> present() const override;

    optional<SegmentPrefix> segmentPrefix;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;

    static const map<SegmentPrefix, OperandMask::Mask> prefixToSegRegMap;
};

class DataSentence : public Sentence {
public:
    typedef Integer Operand;
    typedef tuple<Operand, CodePosition> OperandContainer;
    typedef InstructionNS::DataIdentifier DataIdentifier;

    inline DataSentence(CodePosition pos, Assume assume, DataIdentifier dataIdentifier, vector<OperandContainer> operandContainerVector) :
        Sentence(pos, assume),
        dataIdentifier(dataIdentifier),
        operandContainerVector(operandContainerVector)
    {}

    virtual vector<vector<uchar>> compute() const override;
    virtual tuple<string, vector<string>> present() const override;

    DataIdentifier dataIdentifier;
    vector<OperandContainer> operandContainerVector;
};

struct SentencesSegment {
    string segName;
    vector<shared_ptr<Sentence>> sentences;
};

class RawSentence;
struct RawSentencesSegment;

vector<SentencesSegment> constructSentences(const vector<RawSentencesSegment> &rawSentencesSegmentContainerVector);

#endif
