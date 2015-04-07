#include "TSParser.h"

#include "TSException.h"
#include "TSDiagnostics.h"
#include <functional>

auto constructPseudoSentences(const vector<TSSegmentTokenContainer> &segmentTokenContainerVector)
{
    typedef vector<TSTokenContainer>::const_iterator ItType;
    
    std::function<ItType(ItType, ItType)> locateMemoryBrackets = [](ItType it, ItType endIt) -> ItType {
        while ((it != endIt) &&
               (it->token.type() == TSToken::Type::MEMORY_BRACKET) &&
               (it->token.value<TSToken::MemoryBracket>() == TSToken::MemoryBracket::OPEN))
        {
            ItType bracketIt = it;
            size_t bracketCount = 1;
            while (bracketCount != 0)
            {
                ++it;
                if (it == endIt)
                    throw TSCompileError("unclosed bracket", *bracketIt);

                if (it->token.type() == TSToken::Type::MEMORY_BRACKET)
                {
                    if (it->token.value<TSToken::MemoryBracket>() == TSToken::MemoryBracket::CLOSE)
                        --bracketCount;
                    else
                        ++bracketCount;
                }
            }

            ++it;
        }

        return it;
    };

    std::function<ItType(ItType, ItType)> locateInstructionOperand = [&locateMemoryBrackets](ItType begin, ItType end) -> ItType {
        auto it = begin;

        if (it == end)
            return it;

        if ((it->token.type() == TSToken::Type::REGISTER_8) ||
            (it->token.type() == TSToken::Type::REGISTER_16) ||
            (it->token.type() == TSToken::Type::REGISTER_32) ||
            ((it->token.type() == TSToken::Type::REGISTER_SEGMENT) &&
             (((it + 1) == end) ||
              ((it + 1)->token.type() != TSToken::Type::COLON))))
        {
            return it + 1;
        }

        if ((it->token.type() == TSToken::Type::CONSTANT_NUMBER) ||
            (it->token.type() == TSToken::Type::MATH_SYMBOL))
        {
            auto mathEndIt = getMathTokenSequence(it, end);
            if ((mathEndIt == end) ||
                (mathEndIt->token.type() != TSToken::Type::MEMORY_BRACKET) ||
                (mathEndIt->token.value<TSToken::MemoryBracket>() != TSToken::MemoryBracket::OPEN))
            {
                return mathEndIt;
            }
        }

        if (it->token.type() == TSToken::Type::SIZE_IDENTIFIER)
        {
            ++it;
            if (it->token.type() == TSToken::Type::SIZE_OPERATOR)
                ++it;
        }

        if ((it != end) && (it->token.type() == TSToken::Type::REGISTER_SEGMENT) &&
            ((it + 1) != end) && ((it + 1)->token.type() == TSToken::Type::COLON))
        {
            it += 2;
        }

        auto memoryBracketsEndIt = locateMemoryBrackets(it, end);
        if (memoryBracketsEndIt != it)
            return memoryBracketsEndIt;

        if (it->token.type() == TSToken::Type::USER_IDENTIFIER)
            return locateMemoryBrackets(it + 1, end);

        if ((it->token.type() == TSToken::Type::CONSTANT_NUMBER) ||
            (it->token.type() == TSToken::Type::MATH_SYMBOL))
        {
            auto mathEndIt = getMathTokenSequence(it, end);
            auto memoryBracketsEndIt = locateMemoryBrackets(mathEndIt, end);
            if (mathEndIt == memoryBracketsEndIt)
                throw TSCompileError("just number cannot be address", *it);

            return memoryBracketsEndIt;
        }

        if (it != begin)
            throw TSCompileError("uncomplete operand", *(it - 1));

        return it;
    };

    auto locateDataOperand = [](ItType begin, ItType end) -> ItType {
        auto it = begin;

        if (it == end)
            return it;

        if ((it->token.type() == TSToken::Type::CONSTANT_NUMBER) ||
            (it->token.type() == TSToken::Type::MATH_SYMBOL))
        {
            return getMathTokenSequence(it, end);
        }

        if ((it->token.type() == TSToken::Type::USER_IDENTIFIER) ||
            (it->token.type() == TSToken::Type::CONSTANT_STRING))
        {
            return it + 1;
        }

        return it;
    };

    vector<TSSegmentPseudoSentence> segmentPseudoSentenceVector;
    typedef tuple<LabelType, TSToken::DataIdentifier, size_t> labelParamType;
    map<string, labelParamType> labelMap;

    for (auto segIt = segmentTokenContainerVector.begin(); segIt != segmentTokenContainerVector.end(); ++segIt)
    {
        vector<TSPseudoSentence> pseudoSentenceVector;

        auto beginIt = segIt->tokenContainerVector.begin();
        auto endIt = segIt->tokenContainerVector.end();
        auto it = beginIt;
        while (it != endIt)
        {
            if (it->token.type() == TSToken::Type::USER_IDENTIFIER)
            {
                ++it;
                if (it == endIt)
                    throw TSCompileError("must be colon or size identifier", *it);

                if (it->token.type() == TSToken::Type::COLON)
                {
                    if (labelMap.count((it - 1)->token.value<string>()))
                        throw TSCompileError("duplicate label", *(it - 1));

                    labelMap[(it - 1)->token.value<string>()] = labelParamType(LabelType::LABEL,
                                                                               TSToken::DataIdentifier(),
                                                                               pseudoSentenceVector.size());
                    
                    ++it;
                }
                else if (it->token.type() == TSToken::Type::DATA_IDENTIFIER)
                {
                    if (labelMap.count((it - 1)->token.value<string>()))
                        throw TSCompileError("duplicate label", *(it - 1));

                    labelMap[(it - 1)->token.value<string>()] = labelParamType(LabelType::DATA,
                                                                               it->token.value<TSToken::DataIdentifier>(),
                                                                               pseudoSentenceVector.size());
                }
                else
                    throw TSCompileError("must be colon or size identifier", *(it - 1));
            }
            else if ((it->token.type() == TSToken::Type::INSTRUCTION) ||
                     (it->token.type() == TSToken::Type::DATA_IDENTIFIER))
            {
                TSPseudoSentence pseudoSentence;
                pseudoSentence.baseTokenContainer = *it;
                std::function<ItType(ItType, ItType)> locateOperand = (it->token.type() == TSToken::Type::INSTRUCTION) ? locateInstructionOperand : locateDataOperand;
                ++it;

                auto operandEndIt = locateOperand(it, endIt);
                if (operandEndIt != it)
                {
                    pseudoSentence.operandVector.push_back(vector<TSTokenContainer>(it, operandEndIt));
                    it = operandEndIt;

                    while ((it != endIt) && (it->token.type() == TSToken::Type::COMMA))
                    {
                        ++it;
                        if ((it == endIt) || (locateOperand(it, endIt) == it))
                            throw TSCompileError("no operand after comma", *(it - 1));

                        auto operandEndIt = locateOperand(it, endIt);
                        pseudoSentence.operandVector.push_back(vector<TSTokenContainer>(it, operandEndIt));
                        it = operandEndIt;
                    }
                }

                if ((it != endIt) &&
                    (pseudoSentence.operandVector.size() == 1) &&
                    (pseudoSentence.operandVector[0].size() == 1) &&
                    (pseudoSentence.operandVector[0][0].token.type() == TSToken::Type::USER_IDENTIFIER) &&
                    ((it->token.type() == TSToken::Type::COLON) ||
                     ((it->token.type() == TSToken::Type::DATA_IDENTIFIER) &&
                      (pseudoSentence.operandVector[0][0].row != pseudoSentence.baseTokenContainer.row))))
                {
                    pseudoSentence.operandVector.clear();
                    --it;
                }

                pseudoSentenceVector.push_back(pseudoSentence);
            }
            else
                throw TSCompileError("an instruction, data identifier or label expected", *it);
        }

        segmentPseudoSentenceVector.push_back({segIt->name, pseudoSentenceVector});
    }

    return tuple<decltype(segmentPseudoSentenceVector), decltype(labelMap)>(segmentPseudoSentenceVector, labelMap);
}

void parse(const vector<TSSegmentTokenContainer> &segmentTokenContainerVector)
{
    auto pseudoSentenceResult = constructPseudoSentences(segmentTokenContainerVector);
    printPseudoLabelTable(std::get<1>(pseudoSentenceResult));
    printPseudoSentenceTable(std::get<0>(pseudoSentenceResult));
}
