#ifndef _TSRAWSENTENCE_H_
#define _TSRAWSENTENCE_H_

#include "TSGlobal.h"
#include "TSInteger.h"
#include "TSCodePosition.h"
#include "TSInstruction.h"
#include "TSPseudoSentence.h"
#include "TSUniquePtr.h"
#include "TSSentence.h"

struct TSRawNumber
{
    TSInteger num;
    optional<string> labelStr;
};

class TSSentenceSize;

class TSRawSentence
{
public:
    virtual tuple<string, vector<string>> present() const = 0;
    
    inline TSRawSentence(TSCodePosition pos) :
        _pos(pos)
    {}

    TSCodePosition pos() const;
private:
    TSCodePosition _pos;
};

class TSRawInstructionSentence : public TSRawSentence
{
public:
    typedef TSInstructionSentence::Prefix Prefix;

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

    friend shared_ptr<TSSentence> constructSentenceFromRaw(const TSRawSentence &rawSentence);
    friend vector<TSSentencesSegmentContainer> constructSentences(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainer,
                                                                  const map<string, TSLabelParamType> &labelMap);
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

    friend shared_ptr<TSSentence> constructSentenceFromRaw(const TSRawSentence &rawSentence);
    friend vector<TSSentencesSegmentContainer> constructSentences(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainer,
                                                                  const map<string, TSLabelParamType> &labelMap);
};

typedef tuple<string, vector<shared_ptr<TSRawSentence>>> TSRawSentencesSegmentContainer;

vector<TSRawSentencesSegmentContainer> constructRawSentences(const vector<TSPseudoSentencesSegmentContainer> &pseudoSentencesSegmentContainerVector);

#endif
