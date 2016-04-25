#ifndef _RAWSENTENCE_H_
#define _RAWSENTENCE_H_

#include "Global.h"
#include "Integer.h"
#include "CodePosition.h"
#include "Instruction.h"
#include "PseudoSentence.h"
#include "UniquePtr.h"
#include "Sentence.h"

struct RawNumber {
    Integer num;
    optional<Label> label;
    bool isNotFinal;
};

class SentenceSize;

class RawSentence {
public:
    virtual tuple<string, vector<string>> present(const map<string, Label> &labelMap) const = 0;
    
    inline RawSentence(CodePosition pos, Assume assume) :
        _pos(pos),
        _assume(assume)
    {}

    inline CodePosition pos() const {
        return _pos;
    }

    inline Assume assume() const {
        return _assume;
    }
private:
    CodePosition _pos;
    Assume _assume;
};

class RawInstructionSentence : public RawSentence {
public:
    typedef InstructionSentence::SegmentPrefix SegmentPrefix;

    class Operand {
    public:
        string present(const map<string, Label> &labelMap) const;
        OperandMask::Mask mask;
        RawNumber rawNum;
    };

    typedef InstructionNS::Instruction Instruction;
    typedef tuple<Operand, CodePosition> OperandContainer;

    RawInstructionSentence(const PseudoSentence &pseudoSentence, const map<string, Label> &labelMap);
    virtual tuple<string, vector<string>> present(const map<string, Label> &labelMap) const override;
private:
    optional<SegmentPrefix> segmentPrefix;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;

    friend shared_ptr<Sentence> constructSentenceFromRaw(const RawSentence &rawSentence, vector<bool> linkVector);
    friend vector<SentencesSegment> constructSentences(const vector<RawSentencesSegment> &rawSentencesSegmentContainer);
};

class RawDataSentence : public RawSentence {
public:
    typedef RawNumber Operand;
    typedef tuple<Operand, CodePosition> OperandContainer;
    typedef InstructionNS::DataIdentifier DataIdentifier;

    RawDataSentence(const PseudoSentence &pseudoSentence, const map<string, Label> &labelMap);
    virtual tuple<string, vector<string>> present(const map<string, Label> &labelMap) const override;
private:
    DataIdentifier dataIdentifier;
    vector<OperandContainer> operandContainerVector;

    friend shared_ptr<Sentence> constructSentenceFromRaw(const RawSentence &rawSentence, vector<bool> linkVector);
    friend vector<SentencesSegment> constructSentences(const vector<RawSentencesSegment> &rawSentencesSegmentContainer);
};

struct RawSentencesSegment {
    string segName;
    vector<shared_ptr<RawSentence>> rawSentences;
};

RawInstructionSentence::SegmentPrefix getSegmentOverridePrefix(Token::Register reg);

vector<RawSentencesSegment> constructRawSentences(const vector<PseudoSentencesSegment> &pseudoSentencesSegmentContainerVector,
                                                    const map<string, Label> &labelMap);

#endif
