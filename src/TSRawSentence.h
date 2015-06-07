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
    optional<TSLabel> label;
};

class TSSentenceSize;

class TSRawSentence
{
public:
    virtual tuple<string, vector<string>> present(const map<string, TSLabel> &labelMap) const = 0;
    
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
        string present(const map<string, TSLabel> &labelMap) const;
        TSOperandMask::Mask mask;
        TSRawNumber rawNum;
    };

    typedef TSInstruction::Instruction Instruction;
    typedef tuple<Operand, TSCodePosition> OperandContainer;

    TSRawInstructionSentence(const TSPseudoSentence &pseudoSentence, const map<string, TSLabel> &labelMap);
    virtual tuple<string, vector<string>> present(const map<string, TSLabel> &labelMap) const override;
private:
    vector<Prefix> prefixVector;
    Instruction instruction;
    vector<OperandContainer> operandContainerVector;

    friend shared_ptr<TSSentence> constructSentenceFromRaw(const TSRawSentence &rawSentence, vector<bool> linkVector);
    friend vector<TSSentencesSegmentContainer> constructSentences(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainer);
};

class TSRawDataSentence : public TSRawSentence
{
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
    friend vector<TSSentencesSegmentContainer> constructSentences(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainer);
};

typedef tuple<string, vector<shared_ptr<TSRawSentence>>> TSRawSentencesSegmentContainer;

vector<TSRawSentencesSegmentContainer> constructRawSentences(const vector<TSPseudoSentencesSegmentContainer> &pseudoSentencesSegmentContainerVector,
                                                             const map<string, TSLabel> &labelMap);

#endif
