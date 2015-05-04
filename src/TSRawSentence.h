#ifndef _TSRAWSENTENCE_H_
#define _TSRAWSENTENCE_H_

#include "TSGlobal.h"
#include "TSInteger.h"
#include "TSCodePosition.h"
#include "TSInstruction.h"
#include "TSPseudoSentence.h"
#include "TSUniquePtr.h"

struct TSRawNumber
{
    TSInteger num;
    optional<string> labelStr;
};

class TSRawSentence
{
public:
    virtual tuple<string, vector<string>> present() const = 0;
};

class TSRawInstructionSentence : public TSRawSentence
{
public:
    enum class Prefix
    {
        ES,
        CS,
        SS,
        DS,
        FS,
        GS
    };

    class Operand
    {
    public:
        string present() const;
        TSOperandMask::Mask mask;
        TSRawNumber rawNum;
    };

    typedef TSInstruction::Instruction Instruction;
    typedef tuple<Operand, TSCodePosition> OperandContainer;

    TSRawInstructionSentence(const TSPseudoSentence &pseudoSentence);
    virtual tuple<string, vector<string>> present() const override;
private:
    vector<Prefix> prefixVector;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;
};

class TSRawDataSentence : public TSRawSentence
{
public:
    typedef TSRawNumber Operand;
    typedef tuple<Operand, TSCodePosition> OperandContainer;
    typedef TSInstruction::DataIdentifier DataIdentifier;

    TSRawDataSentence(const TSPseudoSentence &pseudoSentence);
    virtual tuple<string, vector<string>> present() const override;
private:
    DataIdentifier dataIdentifier;
    vector<OperandContainer> operandContainerVector;
};

typedef tuple<string, vector<shared_ptr<TSRawSentence>>> TSRawSentencesSegmentContainer;

vector<TSRawSentencesSegmentContainer> constructRawSentences(const vector<TSPseudoSentencesSegmentContainer> &pseudoSentencesSegmentContainerVector);

#endif
