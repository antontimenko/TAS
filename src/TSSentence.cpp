#include "TSSentence.h"

#include "TSRawSentence.h"
#include "TSInstruction.h"
#include "TSException.h"
#include "TSDiagnostics.h"
#include <algorithm>

using namespace TSOperandMask;

vector<TSInstruction::Definition> findSuitableDefinitions(const TSInstructionSentence &instructionSentence)
{
    vector<TSInstruction::Definition> suitableDefinitions;

    for (auto it = TSInstruction::instructionDefinitionVector.begin(); it != TSInstruction::instructionDefinitionVector.end(); ++it)
    {
        if ((instructionSentence.instruction == it->instruction) &&
            (instructionSentence.operandContainerVector.size() == it->operandFullMasks.size()))
        {
            bool isSuitable = true;
            for (size_t i = 0; i < instructionSentence.operandContainerVector.size(); ++i)
            {
                const TSOperandMask::Mask &opMask = get<0>(instructionSentence.operandContainerVector[i]).mask;
                const TSOperandMask::Mask &defOpMask = it->operandFullMasks[i].mask;

                if ((!defOpMask.match(opMask)) ||
                    ((it->operandFullMasks[i].num) &&
                     (*it->operandFullMasks[i].num != get<0>(instructionSentence.operandContainerVector[i]).num)))
                {
                    isSuitable = false;
                    break;
                }
            }

            if (isSuitable)
                suitableDefinitions.push_back(*it);
        }
    }

    return suitableDefinitions;
}

TSInstruction::Definition findMostSuitableDefinition(const TSInstructionSentence &instructionSentence, const vector<TSInstruction::Definition> &suitableDefinitions)
{
    auto getFullSuitableRank = [](const vector<TSInstructionSentence::OperandContainer> &operandContainerVector,
                                  const TSInstruction::Definition definition) -> size_t {
        auto getSuitableRank = [](const TSOperandMask::Mask &baseMask, const TSOperandMask::Mask &mask) -> size_t {
            size_t rank = (baseMask ^ mask).count();
            
            if (mask.match(IMM))
                rank += ((baseMask ^ mask) & S_ANY).count();

            return rank;
        };

        size_t fullSuitableRank = 0;
        
        for (size_t i = 0; i < operandContainerVector.size(); ++i)
            fullSuitableRank += getSuitableRank(definition.operandFullMasks[i].mask, get<0>(operandContainerVector[i]).mask);
        
        return fullSuitableRank;
    };

    if (suitableDefinitions.empty())
        throw TSCompileError("incorrect instruction or operand", instructionSentence.pos);

    for (auto it = suitableDefinitions.begin(); it != suitableDefinitions.end(); ++it)
    {
        for (auto jt = it->operandFullMasks.begin(); jt != it->operandFullMasks.end(); ++jt)
        {
            if (jt->num)
                return *it;
        }
    }

    TSInstruction::Definition mostSuitableDefinition = suitableDefinitions[0];

    const vector<TSInstructionSentence::OperandContainer> &operandContainerVector = instructionSentence.operandContainerVector;
    
    for (size_t i = 1; i < suitableDefinitions.size(); ++i)
    {
        if (getFullSuitableRank(operandContainerVector, suitableDefinitions[i]) < getFullSuitableRank(operandContainerVector, mostSuitableDefinition))
            mostSuitableDefinition = suitableDefinitions[i];
    }

    return mostSuitableDefinition;
}

vector<vector<uchar>> TSInstructionSentence::compute() const
{
    TSInstruction::Definition definition = findMostSuitableDefinition(*this, findSuitableDefinitions(*this));

    return definition.computeFunc(definition, *this);
}

string TSInstructionSentence::Operand::present() const
{
    if (mask.match(UREG) || mask.match(SREG))
        return findByValue(registerMap, mask)->first;
    else if (mask.match(MEM))
    {
        string memStr;
        
        if (mask.match(S8))
            memStr += "BYTE ";
        else if (mask.match(S16))
            memStr += "WORD ";
        else if (mask.match(S32))
            memStr += "DWORD ";

        memStr += "[";

        string innerMemStr;

        if (mask.match(MEM_16))
        {
            if (mask.match(MEM_16_BX_SI))
                innerMemStr += "BX+SI";
            else if (mask.match(MEM_16_BX_DI))
                innerMemStr += "BX+DI";
            else if (mask.match(MEM_16_BP_SI))
                innerMemStr += "BP+SI";
            else if (mask.match(MEM_16_BP_DI))
                innerMemStr += "BP+DI";
            else if (mask.match(MEM_16_SI))
                innerMemStr += "SI";
            else if (mask.match(MEM_16_DI))
                innerMemStr += "DI";
            else if (mask.match(MEM_16_BP))
                innerMemStr += "BP";
            else
                innerMemStr += "BX";
        }
        else
        {
            if (mask.match(MEM_BASE))
            {
                Mask baseReg = (mask & OPVAL_ANY) | UREG32;
                innerMemStr += findByValue(registerMap, baseReg)->first;
            }

            if (mask.match(MEM_32_INDEX))
            {
                if (!innerMemStr.empty())
                    innerMemStr += "+";
                Mask indexReg = ((mask >> 8) & OPVAL_ANY) | UREG32;
                innerMemStr += findByValue(registerMap, indexReg)->first;
                if (mask.match(MEM_32_INDEX_MULT2))
                    innerMemStr += "*2";
                else if (mask.match(MEM_32_INDEX_MULT4))
                    innerMemStr += "*4";
                else if (mask.match(MEM_32_INDEX_MULT8))
                    innerMemStr += "*8";
            }
        }

        if (innerMemStr.empty())
            innerMemStr += num.str();
        else
        {
            if (num != 0)
                innerMemStr += num.str(true);
        }

        memStr += innerMemStr + "]";

        return memStr;
    }
    else if (mask.match(IMM))
    {
        string immStr;
        
        if (mask.match(S8))
            immStr += "BYTE ";
        else if (mask.match(S16))
            immStr += "WORD ";
        else if (mask.match(S32))
            immStr += "DWORD ";

        immStr += num.str();

        return immStr;
    }
    else
    {
        return num.str();
    }
}

const map<TSInstructionSentence::Prefix, TSOperandMask::Mask> TSInstructionSentence::prefixToSegRegMap = {
    {Prefix::ES, ES},
    {Prefix::CS, CS},
    {Prefix::SS, SS},
    {Prefix::DS, DS},
    {Prefix::FS, FS},
    {Prefix::GS, GS}
};

tuple<string, vector<string>> TSInstructionSentence::present() const
{
    string instructionStr = findByValue(TSInstruction::instructionMap, instruction)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
    {
        string operandStr = get<0>(*it).present();
        
        if (!prefixVector.empty())
        {
            if (get<0>(*it).mask.match(MEM))
            {
                Mask segRegPrefix = prefixToSegRegMap.find(prefixVector[0])->second;
                string segRegStr = findByValue(TSOperandMask::registerMap, segRegPrefix)->first;
                operandStr = segRegStr + ':' + operandStr;
            }
        }

        operandStrVector.push_back(operandStr);
    }

    return make_tuple(instructionStr, operandStrVector);
}

vector<vector<uchar>> TSDataSentence::compute() const
{
    vector<vector<uchar>> res;

    TSInteger::Size intSize;
    switch (dataIdentifier)
    {
    case DataIdentifier::DB:
        intSize = TSInteger::Size::S_8;
        break;
    case DataIdentifier::DW:
        intSize = TSInteger::Size::S_16;
        break;
    case DataIdentifier::DD:
        intSize = TSInteger::Size::S_32;
        break;
    }

    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
    {
        vector<uchar> opRes = get<0>(*it).getCharArrayUnsigned(intSize);
        res.push_back(opRes);
    }

    return res;
}

tuple<string, vector<string>> TSDataSentence::present() const
{
    string instructionStr = findByValue(TSInstruction::dataIdentifierMap, dataIdentifier)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
        operandStrVector.push_back(get<0>(*it).str());

    return make_tuple(instructionStr, operandStrVector);
}

shared_ptr<TSSentence> constructSentenceFromRaw(const TSRawSentence &rawSentence, vector<bool> linkVector = vector<bool>())
{
    if (typeid(rawSentence) == typeid(TSRawInstructionSentence))
    {
        const TSRawInstructionSentence &rawInstructionSentence = static_cast<const TSRawInstructionSentence &>(rawSentence);

        vector<TSInstructionSentence::OperandContainer> operandContainerVector;
        for (auto it = rawInstructionSentence.operandContainerVector.begin(); it != rawInstructionSentence.operandContainerVector.end(); ++it)
        {
            const TSRawInstructionSentence::Operand &rawOperand = get<0>(*it);
            bool isLinkable = ((size_t)(it - rawInstructionSentence.operandContainerVector.begin()) < linkVector.size()) ? linkVector[it - rawInstructionSentence.operandContainerVector.begin()] : false;
            
            operandContainerVector.push_back(TSInstructionSentence::OperandContainer({rawOperand.mask, rawOperand.rawNum.num, isLinkable}, get<1>(*it)));
        }

        return shared_ptr<TSSentence>(new TSInstructionSentence(rawInstructionSentence.pos(),
                                                                rawInstructionSentence.prefixVector,
                                                                rawInstructionSentence.instruction,
                                                                operandContainerVector));
    }
    else
    {
        const TSRawDataSentence &rawDataSentence = static_cast<const TSRawDataSentence &>(rawSentence);

        vector<TSDataSentence::OperandContainer> operandContainerVector;
        for (auto it = rawDataSentence.operandContainerVector.begin(); it != rawDataSentence.operandContainerVector.end(); ++it)
        {
            const TSRawDataSentence::Operand &rawOperand = get<0>(*it);

            operandContainerVector.push_back(TSDataSentence::OperandContainer(rawOperand.num, get<1>(*it)));
        }

        return shared_ptr<TSSentence>(new TSDataSentence(rawDataSentence.pos(),
                                                         rawDataSentence.dataIdentifier,
                                                         operandContainerVector));
    }
}

vector<TSSentencesSegmentContainer> constructSentences(const vector<TSRawSentencesSegmentContainer> &rawSentencesSegmentContainerVector)
{
    auto instructionOperandImplicitSizeSetter = [](TSInstructionSentence::Operand &operand) {
        if (operand.mask.match(IMM) && (!operand.mask.matchAny(S_ANY)))
            operand.mask |= Mask::operandSizeFromIntegerSize(operand.num.sizeAny());
        else if (operand.mask.match(REL) && (!operand.mask.matchAny(S_ANY)))
            operand.mask |= Mask::operandSizeFromIntegerSize(*operand.num.sizeSigned());
    };

    auto instructionOperandSizeChecker = [](const TSInstructionSentence::OperandContainer &operandContainer) {
        const auto &operand = get<0>(operandContainer);

        if ((operand.mask.match(IMM) && (operand.num.sizeAny() > Mask::integerSizeFromOperandSize(operand.mask))) ||
            (operand.mask.match(REL) && (*operand.num.sizeSigned() > Mask::integerSizeFromOperandSize(operand.mask))))
            throw TSCompileError("overflow (constant size too large)", get<1>(operandContainer));
    };

    auto dataOperandSizeChecker = [](const TSDataSentence::OperandContainer &operandContainer,
                                     TSDataSentence::DataIdentifier dataIdentifier) {
        TSInteger::Size suitableSize;
        switch (dataIdentifier)
        {
        case TSDataSentence::DataIdentifier::DB:
            suitableSize = TSInteger::Size::S_8;
            break;
        case TSDataSentence::DataIdentifier::DW:
            suitableSize = TSInteger::Size::S_16;
            break;
        case TSDataSentence::DataIdentifier::DD:
            suitableSize = TSInteger::Size::S_32;
            break;
        }

        if (get<0>(operandContainer).sizeAny() > suitableSize)
            throw TSCompileError("overflow (constant size too large)", get<1>(operandContainer));
    };

    struct DispsSegmentContainer
    {
        string segName;
        vector<optional<size_t>> dispVector;
    };

    class DispsSegmentFinder
    {
    public:
        inline DispsSegmentFinder(string segName) :
            segName(segName)
        {}

        const string segName;

        inline bool operator()(const DispsSegmentContainer &dispsSegmentContainer) const
        {
            return segName == dispsSegmentContainer.segName;
        }
    };

    class RawSentencesSegmentFinder
    {
    public:
        inline RawSentencesSegmentFinder(string segName) :
            segName(segName)
        {}

        const string segName;

        inline bool operator()(const TSRawSentencesSegmentContainer &rawSentencesSegmentContainer)
        {
            return segName == get<0>(rawSentencesSegmentContainer);
        }
    };

    auto computeDisp = [](vector<optional<size_t>>::const_iterator begin,
                          vector<optional<size_t>>::const_iterator end) -> size_t {
        size_t disp = 0;
        for (auto it = begin; it != end; ++it)
            disp += **it;

        return disp;
    };

    auto getLinkVectorFromRawSentence = [](const TSRawInstructionSentence &rawInstructionSentence) -> vector<bool> {
        vector<bool> res;

        for (auto it = rawInstructionSentence.operandContainerVector.begin(); it != rawInstructionSentence.operandContainerVector.end(); ++it)
            res.push_back((bool)get<0>(*it).rawNum.label);

        return res;
    };

    typedef function<vector<DispsSegmentContainer>(vector<DispsSegmentContainer>, vector<TSRawSentencesSegmentContainer>::const_iterator)> recursiveSizeComputerT;
    recursiveSizeComputerT recursiveSizeComputer = [&](vector<DispsSegmentContainer> dispsSegmentContainerVector,
                                                       vector<TSRawSentencesSegmentContainer>::const_iterator segIt) -> vector<DispsSegmentContainer> {
        for (size_t i = 0; i < get<1>(*segIt).size(); ++i)
        {
            auto dispsSegmentContainerIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                        dispsSegmentContainerVector.end(),
                                                        DispsSegmentFinder(get<0>(*segIt)));
            auto &dispVector = dispsSegmentContainerIt->dispVector;

            if (dispVector[i])
                continue;

            shared_ptr<TSRawSentence> sentencePtr = get<1>(*segIt)[i];
            if (typeid(*sentencePtr) == typeid(TSRawInstructionSentence))
            {
                TSRawInstructionSentence &sourceRawInstructionSentence = static_cast<TSRawInstructionSentence &>(*sentencePtr);
                TSRawInstructionSentence rawInstructionSentence = sourceRawInstructionSentence;
                auto &rawOperandContainerVector = rawInstructionSentence.operandContainerVector;

                if (TSInstruction::jumpInstructionsSet.count(rawInstructionSentence.instruction) &&
                    (rawOperandContainerVector.size() == 1) &&
                    get<0>(rawOperandContainerVector[0]).mask.match(REL))
                {
                    TSRawInstructionSentence::Operand jmpRawOperand = get<0>(rawOperandContainerVector[0]);

                    if (!jmpRawOperand.rawNum.label)
                    {
                        shared_ptr<TSSentence> sentence = constructSentenceFromRaw(rawInstructionSentence);
                        TSInstructionSentence &instructionSentence = static_cast<TSInstructionSentence &>(*sentence);
                        auto &operandContainerVector = instructionSentence.operandContainerVector;
                        auto &jmpOperandContainer = operandContainerVector[0];
                        auto &jmpOperand = get<0>(jmpOperandContainer);
                        
                        instructionOperandImplicitSizeSetter(jmpOperand);

                        dispVector[i] = getInstructionBytePresentSize(instructionSentence.compute());
                    }
                    else
                    {
                        TSLabel labelTo = *jmpRawOperand.rawNum.label;
                        TSLabel labelFrom = {nullopt, i + 1, get<0>(*segIt)};

                        bool sizeComputed = false;
                        for (TSInteger::Size innerRelSize = TSInteger::Size::S_8; innerRelSize != TSInteger::Size::S_64; innerRelSize = TSInteger::nextSize(innerRelSize))
                        {
                            TSRawInstructionSentence innerRawInstructionSentence = rawInstructionSentence;
                            auto &innerRawOperandContainerVector = innerRawInstructionSentence.operandContainerVector;
                            auto &innerRawJmpOperandContainer= innerRawOperandContainerVector[0];
                            auto &innerRawJmpOperand = get<0>(innerRawJmpOperandContainer);

                            innerRawJmpOperand.rawNum.num = TSInteger::getMaxValSigned(innerRelSize);
                            innerRawJmpOperand.rawNum.label = nullopt;

                            shared_ptr<TSSentence> innerSentence = constructSentenceFromRaw(innerRawInstructionSentence);
                            TSInstructionSentence &innerInstructionSentence = static_cast<TSInstructionSentence &>(*innerSentence);
                            auto &innerOperandContainerVector = innerInstructionSentence.operandContainerVector;
                            auto &innerJmpOperandContainer = innerOperandContainerVector[0];
                            auto &innerJmpOperand = get<0>(innerJmpOperandContainer);

                            instructionOperandImplicitSizeSetter(innerJmpOperand);

                            auto innerDispsSegmentContainerVector = dispsSegmentContainerVector;
                            auto innerDispsSegmentContainerIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                                             innerDispsSegmentContainerVector.end(),
                                                                             DispsSegmentFinder(get<0>(*segIt)));
                            auto &innerDispVector = innerDispsSegmentContainerIt->dispVector;

                            if (!findSuitableDefinitions(innerInstructionSentence).empty())
                            {
                                innerDispVector[i] = getInstructionBytePresentSize(innerInstructionSentence.compute());

                                auto innerRawSegIt = std::find_if(rawSentencesSegmentContainerVector.begin(),
                                                                  rawSentencesSegmentContainerVector.end(),
                                                                  RawSentencesSegmentFinder(get<0>(*segIt)));

                                innerDispsSegmentContainerVector = recursiveSizeComputer(innerDispsSegmentContainerVector,
                                                                                         innerRawSegIt);

                                auto innerSegIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                               innerDispsSegmentContainerVector.end(),
                                                               DispsSegmentFinder(get<0>(*segIt)));

                                TSInteger curInnerLabelToDisp = computeDisp(innerSegIt->dispVector.begin(),
                                                                            innerSegIt->dispVector.begin() + labelTo.ptr);
                                TSInteger curInnerLabelFromDisp = computeDisp(innerSegIt->dispVector.begin(),
                                                                              innerSegIt->dispVector.begin() + labelFrom.ptr);
                                TSInteger curInnerRel = curInnerLabelToDisp + jmpRawOperand.rawNum.num - curInnerLabelFromDisp;

                                TSInteger::Size curInnerOpSize = *curInnerRel.sizeSigned();

                                TSInteger::Size curInnerNeedOpSize = innerRelSize;

                                if (curInnerOpSize <= curInnerNeedOpSize)
                                {
                                    dispsSegmentContainerVector = innerDispsSegmentContainerVector;
                                    sizeComputed = true;
                                    break;
                                }
                            }
                        }

                        if (!sizeComputed)
                            throw TSCompileError("incorrect instruction or operand", rawInstructionSentence.pos());
                    }
                }
                else
                {
                    vector<TSRawInstructionSentence::OperandContainer *> rawOperandContainerDependVector;
                    
                    if (TSInstruction::jumpInstructionsSet.count(rawInstructionSentence.instruction) &&
                        (rawOperandContainerVector.size() == 1) &&
                        get<0>(rawOperandContainerVector[0]).mask.match(REL))
                    {
                        TSRawInstructionSentence::Operand &operand = get<0>(rawOperandContainerVector[0]);

                        if (operand.rawNum.label)
                            rawOperandContainerDependVector.push_back(&rawOperandContainerVector[0]);
                    }

                    for (auto it = rawOperandContainerVector.begin(); it != rawOperandContainerVector.end(); ++it)
                    {
                        if (get<0>(*it).rawNum.label)
                            rawOperandContainerDependVector.push_back(&*it);
                    }

                    for (auto it = rawOperandContainerDependVector.begin(); it != rawOperandContainerDependVector.end();)
                    {
                        TSRawInstructionSentence::Operand &operand = get<0>(**it);

                        auto curLabelSegIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                          dispsSegmentContainerVector.end(),
                                                          DispsSegmentFinder(operand.rawNum.label->segName));

                        if ((operand.rawNum.label->ptr == 0) ||
                            (curLabelSegIt->dispVector[operand.rawNum.label->ptr - 1]))
                        {
                            operand.rawNum.num += computeDisp(curLabelSegIt->dispVector.begin(),
                                                              curLabelSegIt->dispVector.begin() + operand.rawNum.label->ptr);
                            operand.rawNum.label = nullopt;

                            it = rawOperandContainerDependVector.erase(it);
                            continue;
                        }

                        if (operand.mask.match(IMM) && operand.mask.matchAny(S_ANY))
                        {
                            operand.rawNum.label = nullopt;

                            it = rawOperandContainerDependVector.erase(it);
                            continue;
                        }

                        ++it;
                    }

                    if (rawOperandContainerDependVector.empty())
                    {
                        shared_ptr<TSSentence> sentence = constructSentenceFromRaw(rawInstructionSentence, getLinkVectorFromRawSentence(sourceRawInstructionSentence));
                        TSInstructionSentence &instructionSentence = static_cast<TSInstructionSentence &>(*sentence);
                        auto &operandContainerVector = instructionSentence.operandContainerVector;
                        
                        for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
                        {
                            instructionOperandImplicitSizeSetter(get<0>(*it));
                            instructionOperandSizeChecker(*it);
                        }

                        dispVector[i] = getInstructionBytePresentSize(instructionSentence.compute());
                    }
                    else
                    {
                        vector<TSLabel> labelDependVector;
                        for (auto it = rawOperandContainerDependVector.begin(); it != rawOperandContainerDependVector.end(); ++it)
                        {
                            if (std::find(labelDependVector.begin(), labelDependVector.end(), *get<0>(**it).rawNum.label) == labelDependVector.end())
                                labelDependVector.push_back(*get<0>(**it).rawNum.label);
                        }

                        vector<TSInteger::Size> operandSizeDependVector(rawOperandContainerDependVector.size(), TSInteger::Size::S_8);

                        bool sizeComputed = false;
                        while (true)
                        {
                            TSRawInstructionSentence innerRawInstructionSentence = rawInstructionSentence;
                            auto &innerRawOperandContainerVector = innerRawInstructionSentence.operandContainerVector;

                            for (auto it = innerRawOperandContainerVector.begin(); it != innerRawOperandContainerVector.end(); ++it)
                            {
                                if (get<0>(*it).rawNum.label)
                                {
                                    auto currLabelDependIt = std::find(labelDependVector.begin(),
                                                                       labelDependVector.end(),
                                                                       *get<0>(*it).rawNum.label);
                                    auto currSize = operandSizeDependVector[currLabelDependIt - labelDependVector.begin()];

                                    if (get<0>(*it).mask.match(MEM))
                                        get<0>(*it).rawNum.num = TSInteger::getMaxValSigned(currSize);
                                    else
                                        get<0>(*it).rawNum.num = TSInteger::getMaxValAny(currSize);
                                    get<0>(*it).rawNum.label = nullopt;
                                }                            
                            }

                            shared_ptr<TSSentence> innerSentence = constructSentenceFromRaw(innerRawInstructionSentence, getLinkVectorFromRawSentence(sourceRawInstructionSentence));
                            TSInstructionSentence &innerInstructionSentence = static_cast<TSInstructionSentence &>(*innerSentence);
                            auto &innerOperandContainerVector = innerInstructionSentence.operandContainerVector;

                            for (auto it = innerOperandContainerVector.begin(); it != innerOperandContainerVector.end(); ++it)
                            {
                                instructionOperandImplicitSizeSetter(get<0>(*it));
                                instructionOperandSizeChecker(*it);
                            }

                            auto innerDispsSegmentContainerVector = dispsSegmentContainerVector;
                            auto innerDispsSegmentContainerIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                                             innerDispsSegmentContainerVector.end(),
                                                                             DispsSegmentFinder(get<0>(*segIt)));
                            auto &innerDispVector = innerDispsSegmentContainerIt->dispVector;
                            
                            bool sizeSuites = false;
                            if (!findSuitableDefinitions(innerInstructionSentence).empty())
                            {
                                innerDispVector[i] = getInstructionBytePresentSize(innerInstructionSentence.compute());

                                for (auto it = labelDependVector.begin(); it != labelDependVector.end(); ++it)
                                {
                                    auto innerRawSegIt = std::find_if(rawSentencesSegmentContainerVector.begin(),
                                                                      rawSentencesSegmentContainerVector.end(),
                                                                      RawSentencesSegmentFinder(it->segName));

                                    innerDispsSegmentContainerVector = recursiveSizeComputer(innerDispsSegmentContainerVector,
                                                                                             innerRawSegIt);
                                }

                                sizeSuites = true;
                                for (auto it = rawOperandContainerDependVector.begin(); it != rawOperandContainerDependVector.end(); ++it)
                                {
                                    auto innerSegIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                                   innerDispsSegmentContainerVector.end(),
                                                                   DispsSegmentFinder(get<0>(**it).rawNum.label->segName));
                                    TSInteger curInnerLabelDisp = computeDisp(innerSegIt->dispVector.begin(),
                                                                              innerSegIt->dispVector.begin() + get<0>(**it).rawNum.label->ptr);
                                    TSInteger curInnerOpNum = get<0>(**it).rawNum.num + curInnerLabelDisp;

                                    TSInteger::Size curInnerOpSize = get<0>(**it).mask.match(MEM) ? *curInnerOpNum.sizeSigned() : curInnerOpNum.sizeAny();

                                    TSInteger::Size curInnerNeedOpSize = operandSizeDependVector[it - rawOperandContainerDependVector.begin()];

                                    if (curInnerOpSize > curInnerNeedOpSize)
                                    {
                                        sizeSuites = false;
                                        break;
                                    }
                                }
                            }

                            if (sizeSuites)
                            {
                                dispsSegmentContainerVector = innerDispsSegmentContainerVector;
                                sizeComputed = true;
                                break;
                            }

                            if ((size_t)std::count(operandSizeDependVector.begin(), operandSizeDependVector.end(), TSInteger::Size::S_64) == operandSizeDependVector.size())
                                break;

                            auto it = operandSizeDependVector.end() - 1;
                            do
                            {
                                if (*it != TSInteger::Size::S_64)
                                {
                                    *it = TSInteger::nextSize(*it);
                                    break;
                                }
                                else
                                {
                                    *it = TSInteger::Size::S_8;
                                }
                            } while (it != operandSizeDependVector.begin());
                        }

                        if (!sizeComputed)
                            throw TSCompileError("incorrect instruction or operand", rawInstructionSentence.pos());
                    }
                }
            }
            else
            {
                TSRawDataSentence rawDataSentence = static_cast<TSRawDataSentence &>(*sentencePtr);
                auto &rawOperandContainerVector = rawDataSentence.operandContainerVector;

                size_t mult;
                switch (rawDataSentence.dataIdentifier)
                {
                case TSRawDataSentence::DataIdentifier::DB:
                    mult = 1;
                    break;
                case TSRawDataSentence::DataIdentifier::DW:
                    mult = 2;
                    break;
                case TSRawDataSentence::DataIdentifier::DD:
                    mult = 4;
                    break;
                }

                dispVector[i] = mult * rawOperandContainerVector.size();
            }
        }

        return dispsSegmentContainerVector;
    };

    vector<DispsSegmentContainer> dispsSegmentContainerVector;
    for (auto it = rawSentencesSegmentContainerVector.begin(); it != rawSentencesSegmentContainerVector.end(); ++it)
        dispsSegmentContainerVector.push_back({get<0>(*it), vector<optional<size_t>>(get<1>(*it).size(), nullopt)});

    for (auto it = rawSentencesSegmentContainerVector.begin(); it != rawSentencesSegmentContainerVector.end(); ++it)
        dispsSegmentContainerVector = recursiveSizeComputer(dispsSegmentContainerVector, it);

    vector<TSSentencesSegmentContainer> sentencesSegmentContainer;

    for (auto it = rawSentencesSegmentContainerVector.begin(); it != rawSentencesSegmentContainerVector.end(); ++it)
    {
        vector<shared_ptr<TSSentence>> sentenceVector;
        const string &segName = get<0>(*it);
        const vector<shared_ptr<TSRawSentence>> &rawSentenceVector = get<1>(*it);

        for (auto jt = rawSentenceVector.begin(); jt != rawSentenceVector.end(); ++jt)
        {
            if (typeid(**jt) == typeid(TSRawInstructionSentence))
            {
                const TSRawInstructionSentence &sourceRawInstructionSentence = static_cast<const TSRawInstructionSentence &>(**jt);
                TSRawInstructionSentence rawInstructionSentence = sourceRawInstructionSentence;
                auto &rawOperandContainerVector = rawInstructionSentence.operandContainerVector;

                for (auto kt = rawOperandContainerVector.begin(); kt != rawOperandContainerVector.end(); ++kt)
                {
                    TSRawInstructionSentence::OperandContainer &rawOperandContainer = *kt;
                    TSRawInstructionSentence::Operand &rawOperand = get<0>(rawOperandContainer);

                    if (rawOperand.rawNum.label)
                    {
                        auto labelSegIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                       dispsSegmentContainerVector.end(),
                                                       DispsSegmentFinder(rawOperand.rawNum.label->segName));
                        TSInteger labelVal = computeDisp(labelSegIt->dispVector.begin(),
                                                         labelSegIt->dispVector.begin() + rawOperand.rawNum.label->ptr);

                        rawOperand.rawNum.num += labelVal;
                        rawOperand.rawNum.label = nullopt;

                        if (rawOperand.mask.match(REL))
                        {
                            rawOperand.rawNum.num -= computeDisp(labelSegIt->dispVector.begin(),
                                                                 labelSegIt->dispVector.begin() + (jt - rawSentenceVector.begin()) + 1);
                        }
                    }
                }

                shared_ptr<TSSentence> sentence = constructSentenceFromRaw(rawInstructionSentence, getLinkVectorFromRawSentence(sourceRawInstructionSentence));

                TSInstructionSentence &instructionSentence = static_cast<TSInstructionSentence &>(*sentence);
                auto &operandContainerVector = instructionSentence.operandContainerVector;

                for (auto kt = operandContainerVector.begin(); kt != operandContainerVector.end(); ++kt)
                {
                    instructionOperandImplicitSizeSetter(get<0>(*kt));
                    instructionOperandSizeChecker(*kt);
                }

                sentenceVector.push_back(sentence);
            }
            else
            {
                TSRawDataSentence rawDataSentence = static_cast<const TSRawDataSentence &>(**jt);
                auto &rawOperandContainerVector = rawDataSentence.operandContainerVector;

                for (auto kt = rawOperandContainerVector.begin(); kt != rawOperandContainerVector.end(); ++kt)
                {
                    TSRawDataSentence::OperandContainer &rawOperandContainer = *kt;
                    TSRawDataSentence::Operand &rawOperand = get<0>(rawOperandContainer);

                    if (rawOperand.label)
                    {
                        auto labelSegIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                       dispsSegmentContainerVector.end(),
                                                       DispsSegmentFinder(rawOperand.label->segName));

                        TSInteger labelVal = computeDisp(labelSegIt->dispVector.begin(),
                                                         labelSegIt->dispVector.begin() + rawOperand.label->ptr);
                    
                        rawOperand.num += labelVal;
                        rawOperand.label = nullopt;
                    }
                }

                shared_ptr<TSSentence> sentence = constructSentenceFromRaw(rawDataSentence);

                TSDataSentence &dataSentence = static_cast<TSDataSentence &>(*sentence);
                auto &operandContainerVector = dataSentence.operandContainerVector;

                for (auto kt = operandContainerVector.begin(); kt != operandContainerVector.end(); ++kt)
                    dataOperandSizeChecker(*kt, dataSentence.dataIdentifier);

                sentenceVector.push_back(sentence);
            }
        }

        sentencesSegmentContainer.push_back(TSSentencesSegmentContainer(segName, sentenceVector));
    }

    return sentencesSegmentContainer;
}
