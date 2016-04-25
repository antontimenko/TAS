#include "Sentence.h"

#include "RawSentence.h"
#include "Instruction.h"
#include "Exception.h"
#include "Diagnostics.h"
#include <algorithm>

using namespace OperandMask;

vector<InstructionNS::Definition> findSuitableDefinitions(const InstructionSentence &instructionSentence) {
    vector<InstructionNS::Definition> suitableDefinitions;

    for (auto it = InstructionNS::instructionDefinitionVector.begin(); it != InstructionNS::instructionDefinitionVector.end(); ++it) {
        if ((instructionSentence.instruction == it->instruction) &&
            (instructionSentence.operandContainerVector.size() == it->operandFullMasks.size()))
        {
            bool isSuitable = true;
            for (size_t i = 0; i < instructionSentence.operandContainerVector.size(); ++i) {
                const OperandMask::Mask &opMask = get<0>(instructionSentence.operandContainerVector[i]).mask;
                const OperandMask::Mask &defOpMask = it->operandFullMasks[i].mask;

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

InstructionNS::Definition findMostSuitableDefinition(const InstructionSentence &instructionSentence, const vector<InstructionNS::Definition> &suitableDefinitions) {
    auto getFullSuitableRank = [](const vector<InstructionSentence::OperandContainer> &operandContainerVector,
                                  const InstructionNS::Definition definition) -> size_t {
        auto getSuitableRank = [](const OperandMask::Mask &baseMask, const OperandMask::Mask &mask) -> size_t {
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
        throw CompileError("incorrect instruction or operand", instructionSentence.pos);

    for (auto it = suitableDefinitions.begin(); it != suitableDefinitions.end(); ++it) {
        for (auto jt = it->operandFullMasks.begin(); jt != it->operandFullMasks.end(); ++jt) {
            if (jt->num)
                return *it;
        }
    }

    InstructionNS::Definition mostSuitableDefinition = suitableDefinitions[0];

    const vector<InstructionSentence::OperandContainer> &operandContainerVector = instructionSentence.operandContainerVector;
    
    for (size_t i = 1; i < suitableDefinitions.size(); ++i) {
        if (getFullSuitableRank(operandContainerVector, suitableDefinitions[i]) < getFullSuitableRank(operandContainerVector, mostSuitableDefinition))
            mostSuitableDefinition = suitableDefinitions[i];
    }

    return mostSuitableDefinition;
}

vector<vector<uchar>> InstructionSentence::compute() const {
    InstructionNS::Definition definition = findMostSuitableDefinition(*this, findSuitableDefinitions(*this));

    return definition.computeFunc(definition, *this);
}

string InstructionSentence::Operand::present() const {
    if (mask.match(UREG) || mask.match(SREG))
        return findByValue(registerMap, mask)->first;
    else if (mask.match(MEM)) {
        string memStr;
        
        if (mask.match(S8))
            memStr += "BYTE ";
        else if (mask.match(S16))
            memStr += "WORD ";
        else if (mask.match(S32))
            memStr += "DWORD ";

        memStr += "[";

        string innerMemStr;

        if (mask.match(MEM_16)) {
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
        } else {
            if (mask.match(MEM_BASE)) {
                Mask baseReg = (mask & OPVAL_ANY) | UREG32;
                innerMemStr += findByValue(registerMap, baseReg)->first;
            }

            if (mask.match(MEM_32_INDEX)) {
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
        else {
            if (num != 0)
                innerMemStr += num.str(true);
        }

        memStr += innerMemStr + "]";

        return memStr;
    } else if (mask.match(IMM)) {
        string immStr;
        
        if (mask.match(S8))
            immStr += "BYTE ";
        else if (mask.match(S16))
            immStr += "WORD ";
        else if (mask.match(S32))
            immStr += "DWORD ";

        immStr += num.str();

        return immStr;
    } else
        return num.str();
}

const map<InstructionSentence::SegmentPrefix, OperandMask::Mask> InstructionSentence::prefixToSegRegMap = {
    {SegmentPrefix::ES, ES},
    {SegmentPrefix::CS, CS},
    {SegmentPrefix::SS, SS},
    {SegmentPrefix::DS, DS},
    {SegmentPrefix::FS, FS},
    {SegmentPrefix::GS, GS}
};

tuple<string, vector<string>> InstructionSentence::present() const {
    string instructionStr = findByValue(InstructionNS::instructionMap, instruction)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it) {
        string operandStr = get<0>(*it).present();
        
        if (segmentPrefix) {
            if (get<0>(*it).mask.match(MEM)) {
                Mask segRegPrefix = prefixToSegRegMap.find(*segmentPrefix)->second;
                string segRegStr = findByValue(OperandMask::registerMap, segRegPrefix)->first;
                operandStr = segRegStr + ':' + operandStr;
            }
        }

        operandStrVector.push_back(operandStr);
    }

    return make_tuple(instructionStr, operandStrVector);
}

vector<vector<uchar>> DataSentence::compute() const {
    vector<vector<uchar>> res;

    Integer::Size intSize;
    switch (dataIdentifier) {
    case DataIdentifier::DB:
        intSize = Integer::Size::S_8;
        break;
    case DataIdentifier::DW:
        intSize = Integer::Size::S_16;
        break;
    case DataIdentifier::DD:
        intSize = Integer::Size::S_32;
        break;
    }

    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it) {
        vector<uchar> opRes = get<0>(*it).getCharArrayUnsigned(intSize);
        res.push_back(opRes);
    }

    return res;
}

tuple<string, vector<string>> DataSentence::present() const {
    string instructionStr = findByValue(InstructionNS::dataIdentifierMap, dataIdentifier)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
        operandStrVector.push_back(get<0>(*it).str());

    return make_tuple(instructionStr, operandStrVector);
}

shared_ptr<Sentence> constructSentenceFromRaw(const RawSentence &rawSentence, vector<bool> linkVector = vector<bool>()) {
    if (typeid(rawSentence) == typeid(RawInstructionSentence)) {
        const RawInstructionSentence &rawInstructionSentence = static_cast<const RawInstructionSentence &>(rawSentence);

        vector<InstructionSentence::OperandContainer> operandContainerVector;
        for (auto it = rawInstructionSentence.operandContainerVector.begin(); it != rawInstructionSentence.operandContainerVector.end(); ++it) {
            const RawInstructionSentence::Operand &rawOperand = get<0>(*it);
            bool isLinkable = ((size_t)(it - rawInstructionSentence.operandContainerVector.begin()) < linkVector.size()) ? linkVector[it - rawInstructionSentence.operandContainerVector.begin()] : false;
            
            optional<string> segName = nullopt;
            if (rawOperand.rawNum.label)
                segName = (*rawOperand.rawNum.label).segName;

            operandContainerVector.push_back(InstructionSentence::OperandContainer({rawOperand.mask, rawOperand.rawNum.num, isLinkable, segName}, get<1>(*it)));
        }

        return shared_ptr<Sentence>(new InstructionSentence(rawInstructionSentence.pos(),
                                                                rawInstructionSentence.assume(),
                                                                rawInstructionSentence.segmentPrefix,
                                                                rawInstructionSentence.instruction,
                                                                operandContainerVector));
    } else {
        const RawDataSentence &rawDataSentence = static_cast<const RawDataSentence &>(rawSentence);

        vector<DataSentence::OperandContainer> operandContainerVector;
        for (auto it = rawDataSentence.operandContainerVector.begin(); it != rawDataSentence.operandContainerVector.end(); ++it) {
            const RawDataSentence::Operand &rawOperand = get<0>(*it);

            operandContainerVector.push_back(DataSentence::OperandContainer(rawOperand.num, get<1>(*it)));
        }

        return shared_ptr<Sentence>(new DataSentence(rawDataSentence.pos(),
                                                         rawDataSentence.assume(),
                                                         rawDataSentence.dataIdentifier,
                                                         operandContainerVector));
    }
}

vector<SentencesSegment> constructSentences(const vector<RawSentencesSegment> &rawSentencesSegmentContainerVector) {
    auto instructionOperandImplicitSizeSetter = [](InstructionSentence::Operand &operand) {
        if (operand.mask.match(IMM) && (!operand.mask.matchAny(S_ANY)))
            operand.mask |= Mask::operandSizeFromIntegerSize(operand.num.sizeAny());
        else if (operand.mask.match(REL) && (!operand.mask.matchAny(S_ANY)))
            operand.mask |= Mask::operandSizeFromIntegerSize(*operand.num.sizeSigned());
    };

    auto instructionOperandSizeChecker = [](const InstructionSentence::OperandContainer &operandContainer) {
        const auto &operand = get<0>(operandContainer);

        if ((operand.mask.match(IMM) && (operand.num.sizeAny() > Mask::integerSizeFromOperandSize(operand.mask))) ||
            (operand.mask.match(REL) && (*operand.num.sizeSigned() > Mask::integerSizeFromOperandSize(operand.mask))))
            throw CompileError("overflow (constant size too large)", get<1>(operandContainer));
    };

    auto dataOperandSizeChecker = [](const DataSentence::OperandContainer &operandContainer,
                                     DataSentence::DataIdentifier dataIdentifier) {
        Integer::Size suitableSize;
        switch (dataIdentifier) {
        case DataSentence::DataIdentifier::DB:
            suitableSize = Integer::Size::S_8;
            break;
        case DataSentence::DataIdentifier::DW:
            suitableSize = Integer::Size::S_16;
            break;
        case DataSentence::DataIdentifier::DD:
            suitableSize = Integer::Size::S_32;
            break;
        }

        if (get<0>(operandContainer).sizeAny() > suitableSize)
            throw CompileError("overflow (constant size too large)", get<1>(operandContainer));
    };

    struct DispsSegmentContainer {
        string segName;
        vector<optional<size_t>> dispVector;
    };

    class DispsSegmentFinder {
    public:
        inline DispsSegmentFinder(string segName) :
            segName(segName)
        {}

        const string segName;

        inline bool operator()(const DispsSegmentContainer &dispsSegmentContainer) const {
            return segName == dispsSegmentContainer.segName;
        }
    };

    class RawSentencesSegmentFinder {
    public:
        inline RawSentencesSegmentFinder(string segName) :
            segName(segName)
        {}

        const string segName;

        inline bool operator()(const RawSentencesSegment &rawSentencesSegmentContainer) {
            return segName == rawSentencesSegmentContainer.segName;
        }
    };

    auto computeDisp = [](vector<optional<size_t>>::const_iterator begin,
                          vector<optional<size_t>>::const_iterator end) -> size_t {
        size_t disp = 0;
        for (auto it = begin; it != end; ++it)
            disp += **it;

        return disp;
    };

    auto getLinkVectorFromRawSentence = [](const RawInstructionSentence &rawInstructionSentence) -> vector<bool> {
        vector<bool> res;

        for (auto it = rawInstructionSentence.operandContainerVector.begin(); it != rawInstructionSentence.operandContainerVector.end(); ++it)
            res.push_back((bool)get<0>(*it).rawNum.label);

        return res;
    };

    typedef function<vector<DispsSegmentContainer>(vector<DispsSegmentContainer>, vector<RawSentencesSegment>::const_iterator)> recursiveSizeComputerT;
    recursiveSizeComputerT recursiveSizeComputer = [&](vector<DispsSegmentContainer> dispsSegmentContainerVector,
                                                       vector<RawSentencesSegment>::const_iterator segIt) -> vector<DispsSegmentContainer> {
        for (size_t i = 0; i < segIt->rawSentences.size(); ++i) {
            auto dispsSegmentContainerIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                        dispsSegmentContainerVector.end(),
                                                        DispsSegmentFinder(segIt->segName));
            auto &dispVector = dispsSegmentContainerIt->dispVector;

            if (dispVector[i])
                continue;

            shared_ptr<RawSentence> sentencePtr = segIt->rawSentences[i];
            if (typeid(*sentencePtr) == typeid(RawInstructionSentence)) {
                RawInstructionSentence &sourceRawInstructionSentence = static_cast<RawInstructionSentence &>(*sentencePtr);
                RawInstructionSentence rawInstructionSentence = sourceRawInstructionSentence;
                auto &rawOperandContainerVector = rawInstructionSentence.operandContainerVector;

                if (InstructionNS::jumpInstructionsSet.count(rawInstructionSentence.instruction) &&
                    (rawOperandContainerVector.size() == 1) &&
                    get<0>(rawOperandContainerVector[0]).mask.match(REL))
                {
                    RawInstructionSentence::Operand jmpRawOperand = get<0>(rawOperandContainerVector[0]);

                    if (!jmpRawOperand.rawNum.isNotFinal) {
                        shared_ptr<Sentence> sentence = constructSentenceFromRaw(rawInstructionSentence);
                        InstructionSentence &instructionSentence = static_cast<InstructionSentence &>(*sentence);
                        auto &operandContainerVector = instructionSentence.operandContainerVector;
                        auto &jmpOperandContainer = operandContainerVector[0];
                        auto &jmpOperand = get<0>(jmpOperandContainer);
                        
                        instructionOperandImplicitSizeSetter(jmpOperand);

                        dispVector[i] = getInstructionBytePresentSize(instructionSentence.compute());
                    } else {
                        Label labelTo = *jmpRawOperand.rawNum.label;
                        Label labelFrom = {nullopt, i + 1, segIt->segName};

                        bool sizeComputed = false;
                        for (Integer::Size innerRelSize = Integer::Size::S_8; innerRelSize != Integer::Size::S_64; innerRelSize = Integer::nextSize(innerRelSize)) {
                            RawInstructionSentence innerRawInstructionSentence = rawInstructionSentence;
                            auto &innerRawOperandContainerVector = innerRawInstructionSentence.operandContainerVector;
                            auto &innerRawJmpOperandContainer= innerRawOperandContainerVector[0];
                            auto &innerRawJmpOperand = get<0>(innerRawJmpOperandContainer);

                            innerRawJmpOperand.rawNum.num = Integer::getMaxValSigned(innerRelSize);
                            innerRawJmpOperand.rawNum.isNotFinal = false;

                            shared_ptr<Sentence> innerSentence = constructSentenceFromRaw(innerRawInstructionSentence);
                            InstructionSentence &innerInstructionSentence = static_cast<InstructionSentence &>(*innerSentence);
                            auto &innerOperandContainerVector = innerInstructionSentence.operandContainerVector;
                            auto &innerJmpOperandContainer = innerOperandContainerVector[0];
                            auto &innerJmpOperand = get<0>(innerJmpOperandContainer);

                            instructionOperandImplicitSizeSetter(innerJmpOperand);

                            auto innerDispsSegmentContainerVector = dispsSegmentContainerVector;
                            auto innerDispsSegmentContainerIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                                             innerDispsSegmentContainerVector.end(),
                                                                             DispsSegmentFinder(segIt->segName));
                            auto &innerDispVector = innerDispsSegmentContainerIt->dispVector;

                            if (!findSuitableDefinitions(innerInstructionSentence).empty()) {
                                innerDispVector[i] = getInstructionBytePresentSize(innerInstructionSentence.compute());

                                auto innerRawSegIt = std::find_if(rawSentencesSegmentContainerVector.begin(),
                                                                  rawSentencesSegmentContainerVector.end(),
                                                                  RawSentencesSegmentFinder(segIt->segName));

                                innerDispsSegmentContainerVector = recursiveSizeComputer(innerDispsSegmentContainerVector,
                                                                                         innerRawSegIt);

                                auto innerSegIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                               innerDispsSegmentContainerVector.end(),
                                                               DispsSegmentFinder(segIt->segName));

                                Integer curInnerLabelToDisp = computeDisp(innerSegIt->dispVector.begin(),
                                                                            innerSegIt->dispVector.begin() + labelTo.ptr);
                                Integer curInnerLabelFromDisp = computeDisp(innerSegIt->dispVector.begin(),
                                                                              innerSegIt->dispVector.begin() + labelFrom.ptr);
                                Integer curInnerRel = curInnerLabelToDisp + jmpRawOperand.rawNum.num - curInnerLabelFromDisp;

                                Integer::Size curInnerOpSize = *curInnerRel.sizeSigned();

                                Integer::Size curInnerNeedOpSize = innerRelSize;

                                if (curInnerOpSize <= curInnerNeedOpSize) {
                                    dispsSegmentContainerVector = innerDispsSegmentContainerVector;
                                    sizeComputed = true;
                                    break;
                                }
                            }
                        }

                        if (!sizeComputed)
                            throw CompileError("incorrect instruction or operand", rawInstructionSentence.pos());
                    }
                } else {
                    vector<RawInstructionSentence::OperandContainer *> rawOperandContainerDependVector;
                    
                    if (InstructionNS::jumpInstructionsSet.count(rawInstructionSentence.instruction) &&
                        (rawOperandContainerVector.size() == 1) &&
                        get<0>(rawOperandContainerVector[0]).mask.match(REL))
                    {
                        RawInstructionSentence::Operand &operand = get<0>(rawOperandContainerVector[0]);

                        if (operand.rawNum.isNotFinal)
                            rawOperandContainerDependVector.push_back(&rawOperandContainerVector[0]);
                    }

                    for (auto it = rawOperandContainerVector.begin(); it != rawOperandContainerVector.end(); ++it) {
                        if (get<0>(*it).rawNum.isNotFinal)
                            rawOperandContainerDependVector.push_back(&*it);
                    }

                    for (auto it = rawOperandContainerDependVector.begin(); it != rawOperandContainerDependVector.end();) {
                        RawInstructionSentence::Operand &operand = get<0>(**it);

                        auto curLabelSegIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                          dispsSegmentContainerVector.end(),
                                                          DispsSegmentFinder(operand.rawNum.label->segName));

                        if ((operand.rawNum.label->ptr == 0) ||
                            (curLabelSegIt->dispVector[operand.rawNum.label->ptr - 1]))
                        {
                            operand.rawNum.num += computeDisp(curLabelSegIt->dispVector.begin(),
                                                              curLabelSegIt->dispVector.begin() + operand.rawNum.label->ptr);
                            operand.rawNum.isNotFinal = false;

                            it = rawOperandContainerDependVector.erase(it);
                            continue;
                        }

                        if (operand.mask.match(IMM) && operand.mask.matchAny(S_ANY)) {
                            operand.rawNum.isNotFinal = false;

                            it = rawOperandContainerDependVector.erase(it);
                            continue;
                        }

                        ++it;
                    }

                    if (rawOperandContainerDependVector.empty()) {
                        shared_ptr<Sentence> sentence = constructSentenceFromRaw(rawInstructionSentence, getLinkVectorFromRawSentence(sourceRawInstructionSentence));
                        InstructionSentence &instructionSentence = static_cast<InstructionSentence &>(*sentence);
                        auto &operandContainerVector = instructionSentence.operandContainerVector;
                        
                        for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it) {
                            instructionOperandImplicitSizeSetter(get<0>(*it));
                            instructionOperandSizeChecker(*it);
                        }

                        dispVector[i] = getInstructionBytePresentSize(instructionSentence.compute());
                    } else {
                        vector<Label> labelDependVector;
                        for (auto it = rawOperandContainerDependVector.begin(); it != rawOperandContainerDependVector.end(); ++it) {
                            if (std::find(labelDependVector.begin(), labelDependVector.end(), *get<0>(**it).rawNum.label) == labelDependVector.end())
                                labelDependVector.push_back(*get<0>(**it).rawNum.label);
                        }

                        vector<Integer::Size> operandSizeDependVector(rawOperandContainerDependVector.size(), Integer::Size::S_8);

                        bool sizeComputed = false;
                        while (true) {
                            RawInstructionSentence innerRawInstructionSentence = rawInstructionSentence;
                            auto &innerRawOperandContainerVector = innerRawInstructionSentence.operandContainerVector;

                            for (auto it = innerRawOperandContainerVector.begin(); it != innerRawOperandContainerVector.end(); ++it) {
                                if (get<0>(*it).rawNum.isNotFinal) {
                                    auto currLabelDependIt = std::find(labelDependVector.begin(),
                                                                       labelDependVector.end(),
                                                                       *get<0>(*it).rawNum.label);
                                    auto currSize = operandSizeDependVector[currLabelDependIt - labelDependVector.begin()];

                                    if (get<0>(*it).mask.match(MEM))
                                        get<0>(*it).rawNum.num = Integer::getMaxValSigned(currSize);
                                    else
                                        get<0>(*it).rawNum.num = Integer::getMaxValAny(currSize);
                                    get<0>(*it).rawNum.isNotFinal = false;
                                }                            
                            }

                            shared_ptr<Sentence> innerSentence = constructSentenceFromRaw(innerRawInstructionSentence, getLinkVectorFromRawSentence(sourceRawInstructionSentence));
                            InstructionSentence &innerInstructionSentence = static_cast<InstructionSentence &>(*innerSentence);
                            auto &innerOperandContainerVector = innerInstructionSentence.operandContainerVector;

                            for (auto it = innerOperandContainerVector.begin(); it != innerOperandContainerVector.end(); ++it) {
                                instructionOperandImplicitSizeSetter(get<0>(*it));
                                instructionOperandSizeChecker(*it);
                            }

                            auto innerDispsSegmentContainerVector = dispsSegmentContainerVector;
                            auto innerDispsSegmentContainerIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                                             innerDispsSegmentContainerVector.end(),
                                                                             DispsSegmentFinder(segIt->segName));
                            auto &innerDispVector = innerDispsSegmentContainerIt->dispVector;
                            
                            bool sizeSuites = false;
                            if (!findSuitableDefinitions(innerInstructionSentence).empty()) {
                                innerDispVector[i] = getInstructionBytePresentSize(innerInstructionSentence.compute());

                                for (auto it = labelDependVector.begin(); it != labelDependVector.end(); ++it) {
                                    auto innerRawSegIt = std::find_if(rawSentencesSegmentContainerVector.begin(),
                                                                      rawSentencesSegmentContainerVector.end(),
                                                                      RawSentencesSegmentFinder(it->segName));

                                    innerDispsSegmentContainerVector = recursiveSizeComputer(innerDispsSegmentContainerVector,
                                                                                             innerRawSegIt);
                                }

                                sizeSuites = true;
                                for (auto it = rawOperandContainerDependVector.begin(); it != rawOperandContainerDependVector.end(); ++it) {
                                    auto innerSegIt = std::find_if(innerDispsSegmentContainerVector.begin(),
                                                                   innerDispsSegmentContainerVector.end(),
                                                                   DispsSegmentFinder(get<0>(**it).rawNum.label->segName));
                                    Integer curInnerLabelDisp = computeDisp(innerSegIt->dispVector.begin(),
                                                                              innerSegIt->dispVector.begin() + get<0>(**it).rawNum.label->ptr);
                                    Integer curInnerOpNum = get<0>(**it).rawNum.num + curInnerLabelDisp;

                                    Integer::Size curInnerOpSize = get<0>(**it).mask.match(MEM) ? *curInnerOpNum.sizeSigned() : curInnerOpNum.sizeAny();

                                    Integer::Size curInnerNeedOpSize = operandSizeDependVector[it - rawOperandContainerDependVector.begin()];

                                    if (curInnerOpSize > curInnerNeedOpSize) {
                                        sizeSuites = false;
                                        break;
                                    }
                                }
                            }

                            if (sizeSuites) {
                                dispsSegmentContainerVector = innerDispsSegmentContainerVector;
                                sizeComputed = true;
                                break;
                            }

                            if ((size_t)std::count(operandSizeDependVector.begin(), operandSizeDependVector.end(), Integer::Size::S_64) == operandSizeDependVector.size())
                                break;

                            auto it = operandSizeDependVector.end() - 1;
                            do {
                                if (*it != Integer::Size::S_64) {
                                    *it = Integer::nextSize(*it);
                                    break;
                                } else {
                                    *it = Integer::Size::S_8;
                                }
                            } while (it != operandSizeDependVector.begin());
                        }

                        if (!sizeComputed)
                            throw CompileError("incorrect instruction or operand", rawInstructionSentence.pos());
                    }
                }
            } else {
                RawDataSentence rawDataSentence = static_cast<RawDataSentence &>(*sentencePtr);
                auto &rawOperandContainerVector = rawDataSentence.operandContainerVector;

                size_t mult;
                switch (rawDataSentence.dataIdentifier) {
                case RawDataSentence::DataIdentifier::DB:
                    mult = 1;
                    break;
                case RawDataSentence::DataIdentifier::DW:
                    mult = 2;
                    break;
                case RawDataSentence::DataIdentifier::DD:
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
        dispsSegmentContainerVector.push_back({it->segName, vector<optional<size_t>>(it->rawSentences.size(), nullopt)});

    for (auto it = rawSentencesSegmentContainerVector.begin(); it != rawSentencesSegmentContainerVector.end(); ++it)
        dispsSegmentContainerVector = recursiveSizeComputer(dispsSegmentContainerVector, it);

    vector<SentencesSegment> sentencesSegmentContainer;

    for (auto it = rawSentencesSegmentContainerVector.begin(); it != rawSentencesSegmentContainerVector.end(); ++it) {
        vector<shared_ptr<Sentence>> sentenceVector;
        const string &segName = it->segName;
        const vector<shared_ptr<RawSentence>> &rawSentenceVector = it->rawSentences;

        for (auto jt = rawSentenceVector.begin(); jt != rawSentenceVector.end(); ++jt) {
            if (typeid(**jt) == typeid(RawInstructionSentence)) {
                const RawInstructionSentence &sourceRawInstructionSentence = static_cast<const RawInstructionSentence &>(**jt);
                RawInstructionSentence rawInstructionSentence = sourceRawInstructionSentence;
                auto &rawOperandContainerVector = rawInstructionSentence.operandContainerVector;

                for (auto kt = rawOperandContainerVector.begin(); kt != rawOperandContainerVector.end(); ++kt) {
                    RawInstructionSentence::OperandContainer &rawOperandContainer = *kt;
                    RawInstructionSentence::Operand &rawOperand = get<0>(rawOperandContainer);

                    if (rawOperand.rawNum.isNotFinal) {
                        auto labelSegIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                       dispsSegmentContainerVector.end(),
                                                       DispsSegmentFinder(rawOperand.rawNum.label->segName));
                        Integer labelVal = computeDisp(labelSegIt->dispVector.begin(),
                                                         labelSegIt->dispVector.begin() + rawOperand.rawNum.label->ptr);

                        rawOperand.rawNum.num += labelVal;

                        if (rawOperand.mask.match(REL)) {
                            rawOperand.rawNum.num -= computeDisp(labelSegIt->dispVector.begin(),
                                                                 labelSegIt->dispVector.begin() + (jt - rawSentenceVector.begin()) + 1);
                        }
                    }
                }

                shared_ptr<Sentence> sentence = constructSentenceFromRaw(rawInstructionSentence, getLinkVectorFromRawSentence(sourceRawInstructionSentence));

                InstructionSentence &instructionSentence = static_cast<InstructionSentence &>(*sentence);
                auto &operandContainerVector = instructionSentence.operandContainerVector;

                for (auto kt = operandContainerVector.begin(); kt != operandContainerVector.end(); ++kt) {
                    instructionOperandImplicitSizeSetter(get<0>(*kt));
                    instructionOperandSizeChecker(*kt);
                }

                sentenceVector.push_back(sentence);
            } else {
                RawDataSentence rawDataSentence = static_cast<const RawDataSentence &>(**jt);
                auto &rawOperandContainerVector = rawDataSentence.operandContainerVector;

                for (auto kt = rawOperandContainerVector.begin(); kt != rawOperandContainerVector.end(); ++kt) {
                    RawDataSentence::OperandContainer &rawOperandContainer = *kt;
                    RawDataSentence::Operand &rawOperand = get<0>(rawOperandContainer);

                    if (rawOperand.label) {
                        auto labelSegIt = std::find_if(dispsSegmentContainerVector.begin(),
                                                       dispsSegmentContainerVector.end(),
                                                       DispsSegmentFinder(rawOperand.label->segName));

                        Integer labelVal = computeDisp(labelSegIt->dispVector.begin(),
                                                         labelSegIt->dispVector.begin() + rawOperand.label->ptr);
                    
                        rawOperand.num += labelVal;
                    }
                }

                shared_ptr<Sentence> sentence = constructSentenceFromRaw(rawDataSentence);

                DataSentence &dataSentence = static_cast<DataSentence &>(*sentence);
                auto &operandContainerVector = dataSentence.operandContainerVector;

                for (auto kt = operandContainerVector.begin(); kt != operandContainerVector.end(); ++kt)
                    dataOperandSizeChecker(*kt, dataSentence.dataIdentifier);

                sentenceVector.push_back(sentence);
            }
        }

        sentencesSegmentContainer.push_back({segName, sentenceVector});
    }

    return sentencesSegmentContainer;
}
