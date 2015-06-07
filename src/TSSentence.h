#ifndef _TSSENTENCE_H_
#define _TSSENTENCE_H_

#include "TSGlobal.h"
#include "TSInteger.h"
#include "TSOperandMask.h"
#include "TSCodePosition.h"
#include "TSInstruction.h"
#include "TSPseudoSentence.h"

class TSSentence
{
public:
    virtual vector<vector<uchar>> compute() const = 0;
    virtual tuple<string, vector<string>> present() const = 0;
    
    TSSentence(TSCodePosition pos) :
        pos(pos)
    {}

    TSCodePosition pos;
};

class TSInstructionSentence: public TSSentence
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
        TSInteger num;
        bool isLinkable;
    };

    typedef TSInstruction::Instruction Instruction;
    typedef tuple<Operand, TSCodePosition> OperandContainer;

    inline TSInstructionSentence(TSCodePosition pos, vector<Prefix> prefixVector, Instruction instruction, vector<OperandContainer> operandContainerVector) :
        TSSentence(pos),
        prefixVector(prefixVector),
        instruction(instruction),
        operandContainerVector(operandContainerVector)
    {}

    virtual vector<vector<uchar>> compute() const override;
    virtual tuple<string, vector<string>> present() const override;

    vector<Prefix> prefixVector;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;

    static const map<Prefix, TSOperandMask::Mask> prefixToSegRegMap;
};

class TSDataSentence : public TSSentence
{
public:
    typedef TSInteger Operand;
    typedef tuple<Operand, TSCodePosition> OperandContainer;
    typedef TSInstruction::DataIdentifier DataIdentifier;

    inline TSDataSentence(TSCodePosition pos, DataIdentifier dataIdentifier, vector<OperandContainer> operandContainerVector) :
        TSSentence(pos),
        dataIdentifier(dataIdentifier),
        operandContainerVector(operandContainerVector)
    {}

    virtual vector<vector<uchar>> compute() const override;
    virtual tuple<string, vector<string>> present() const override;

    DataIdentifier dataIdentifier;
    vector<OperandContainer> operandContainerVector;
};

typedef tuple<string, vector<shared_ptr<TSSentence>>> TSSentencesSegmentContainer;

class TSRawSentence;
typedef tuple<string, vector<shared_ptr<TSRawSentence>>> TSRawSentencesSegmentContainer;

vector<TSSentencesSegmentContainer> constructSentences(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainerVector);

#endif
