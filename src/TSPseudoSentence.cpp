#include "TSPseudoSentence.h"

#include "TSException.h"
#include "TSInstruction.h"

using namespace TSOperandMask;

tuple<vector<TSPseudoSentencesSegment>, map<string, TSLabel>> splitPseudoSentences(const vector<TSTokenSegment> &segmentTokenContainerVector) {
    typedef vector<TSTokenContainer>::const_iterator ItType;

    auto findSegmentByName = [](string segName, const vector<TSTokenSegment> &tokenSegments) -> vector<TSTokenSegment>::const_iterator {
        for (auto it = tokenSegments.begin(); it != tokenSegments.end(); ++it)
            if (it->segName == segName)
                return it;

        return tokenSegments.end();
    };

    auto locateMemoryBrackets = [](ItType it, ItType endIt) -> ItType {
        while ((it != endIt) &&
               (it->token.type() == TSToken::Type::MEMORY_BRACKET) &&
               (it->token.value<TSToken::MemoryBracket>() == TSToken::MemoryBracket::OPEN))
        {
            ItType bracketIt = it;

            while ((it != endIt) &&
                   ((it->token.type() != TSToken::Type::MEMORY_BRACKET) ||
                    (it->token.value<TSToken::MemoryBracket>() != TSToken::MemoryBracket::CLOSE)))
            {
                ++it;
            }

            if (it == endIt)
                throw TSCompileError("unclosed bracket", bracketIt->pos);

            ++it;
        }

        return it;
    };

    function<ItType(ItType, ItType)> locateInstructionOperand = [&](ItType begin, ItType end) -> ItType {
        auto it = begin;

        if (it == end)
            return it;

        if ((it->token.type() == TSToken::Type::REGISTER) &&
            ((it->token.value<TSToken::Register>().match(UREG)) ||
             ((it->token.value<TSToken::Register>().match(SREG)) &&
              (((it + 1) == end) ||
               ((it + 1)->token.type() != TSToken::Type::COLON)))))
        {
            return it + 1;
        }

        if (it->token.type() == TSToken::Type::SIZE_IDENTIFIER) {
            ++it;
            if (it->token.type() == TSToken::Type::SIZE_OPERATOR)
                ++it;
        }

        if ((it != end) &&
            (it->token.type() == TSToken::Type::REGISTER) &&
            (it->token.value<TSToken::Register>().match(SREG)) &&
            ((it + 1) != end) &&
            ((it + 1)->token.type() == TSToken::Type::COLON))
        {
            it += 2;
        }

        auto operandEndIt = locateMemoryBrackets(getMathTokenSequence(it, end), end);

        if ((operandEndIt == it) && (it != begin))
            throw TSCompileError("incomplete operand", (it - 1)->pos);

        return operandEndIt;
    };

    function<ItType(ItType, ItType)> locateDataOperand = [&](ItType begin, ItType end) -> ItType {
        auto it = begin;

        if (it == end)
            return it;

        if ((it->token.type() == TSToken::Type::CONSTANT_NUMBER) ||
            (it->token.type() == TSToken::Type::MATH_SYMBOL) ||
            (it->token.type() == TSToken::Type::USER_IDENTIFIER))
        {
            return getMathTokenSequence(it, end);
        }

        if (it->token.type() == TSToken::Type::CONSTANT_STRING)
            return it + 1;

        return it;
    };

    vector<TSPseudoSentencesSegment> segmentPseudoSentenceVector;
    map<string, TSLabel> labelMap;

    for (auto segIt = segmentTokenContainerVector.begin(); segIt != segmentTokenContainerVector.end(); ++segIt) {
        vector<TSPseudoSentence> pseudoSentenceVector;

        TSAssume currentAssume;

        auto beginIt = segIt->tokenContainers.begin();
        auto endIt = segIt->tokenContainers.end();
        auto it = beginIt;
        while (it != endIt) {
            if (it->token.type() == TSToken::Type::USER_IDENTIFIER) {
                ++it;
                if (it == endIt)
                    throw TSCompileError("must be colon or size identifier", it->pos);

                if (it->token.type() == TSToken::Type::COLON) {
                    if (labelMap.count((it - 1)->token.value<string>()))
                        throw TSCompileError("duplicate label", (it - 1)->pos);

                    labelMap[(it - 1)->token.value<string>()] = {nullopt,
                                                                 pseudoSentenceVector.size(),
                                                                 segIt->segName};
                    
                    ++it;
                } else if (it->token.type() == TSToken::Type::DATA_IDENTIFIER) {
                    if (labelMap.count((it - 1)->token.value<string>()))
                        throw TSCompileError("duplicate label", (it - 1)->pos);

                    labelMap[(it - 1)->token.value<string>()] = {it->token.value<TSToken::DataIdentifier>(),
                                                                 pseudoSentenceVector.size(),
                                                                 segIt->segName};
                } else
                    throw TSCompileError("must be colon or size identifier", (it - 1)->pos);
            } else if ((it->token.type() == TSToken::Type::INSTRUCTION) ||
                       (it->token.type() == TSToken::Type::DATA_IDENTIFIER))
            {
                TSPseudoSentence pseudoSentence;
                pseudoSentence.baseTokenContainer = *it;
                function<ItType(ItType, ItType)> locateOperand = (it->token.type() == TSToken::Type::INSTRUCTION) ? locateInstructionOperand : locateDataOperand;
                ++it;

                auto operandEndIt = locateOperand(it, endIt);
                if (operandEndIt != it) {
                    pseudoSentence.operandsTokenContainerVector.push_back(vector<TSTokenContainer>(it, operandEndIt));
                    it = operandEndIt;

                    while ((it != endIt) && (it->token.type() == TSToken::Type::COMMA)) {
                        ++it;
                        if ((it == endIt) || (locateOperand(it, endIt) == it))
                            throw TSCompileError("no operand after comma", (it - 1)->pos);

                        auto operandEndIt = locateOperand(it, endIt);
                        pseudoSentence.operandsTokenContainerVector.push_back(vector<TSTokenContainer>(it, operandEndIt));
                        it = operandEndIt;
                    }
                }

                if ((it != endIt) &&
                    (pseudoSentence.operandsTokenContainerVector.size() == 1) &&
                    (pseudoSentence.operandsTokenContainerVector[0].size() == 1) &&
                    (pseudoSentence.operandsTokenContainerVector[0][0].token.type() == TSToken::Type::USER_IDENTIFIER) &&
                    ((it->token.type() == TSToken::Type::COLON) ||
                     ((it->token.type() == TSToken::Type::DATA_IDENTIFIER) &&
                      (pseudoSentence.operandsTokenContainerVector[0][0].pos.row != pseudoSentence.baseTokenContainer.pos.row))))
                {
                    pseudoSentence.operandsTokenContainerVector.clear();
                    --it;
                }

                pseudoSentence.assume = currentAssume;
                pseudoSentenceVector.push_back(pseudoSentence);
            } else if (it->token.type() == TSToken::Type::ASSUME_DIRECTIVE) {
                ++it;
                while (true) {
                    if ((it == segIt->tokenContainers.end()) ||
                        ((it + 1) == segIt->tokenContainers.end()) ||
                        ((it + 2) == segIt->tokenContainers.end()))
                    {
                        throw TSCompileError("unexpected ASSUME end", (segIt->tokenContainers.end() - 1)->pos);
                    }

                    if ((it->token.type() != TSToken::Type::REGISTER) ||
                        (!it->token.value<TSToken::Register>().match(TSOperandMask::SREG)))
                    {
                        throw TSCompileError("must be segment register here", it->pos);
                    }

                    if ((it + 1)->token.type() != TSToken::Type::COLON)
                        throw TSCompileError("must be colon here", (it + 1)->pos);

                    if ((it + 2)->token.type() != TSToken::Type::USER_IDENTIFIER)
                        throw TSCompileError("must be segment name here", (it + 2)->pos);

                    auto currentSegIt = findSegmentByName((it + 2)->token.value<string>(), segmentTokenContainerVector);

                    if (currentSegIt == segmentTokenContainerVector.end())
                        throw TSCompileError("must be segment name here", (it + 2)->pos);

                    currentAssume.setSegment(currentSegIt->segName, it->token.value<TSToken::Register>());

                    if (((it + 3) == segIt->tokenContainers.end()) || ((it + 3)->token.type() != TSToken::Type::COMMA)) {
                        it += 3;
                        break;
                    }

                    it += 4;
                }
            } else
                throw TSCompileError("an instruction, data identifier, assume or label expected", it->pos);
        }

        segmentPseudoSentenceVector.push_back({segIt->segName, pseudoSentenceVector});
    }

    return make_tuple(segmentPseudoSentenceVector, labelMap);
}
