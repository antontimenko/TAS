#include "PseudoSentence.h"

#include "Exception.h"
#include "Instruction.h"

using namespace OperandMask;

tuple<vector<PseudoSentencesSegment>, map<string, Label>> splitPseudoSentences(const vector<TokenSegment> &segmentTokenContainerVector) {
    typedef vector<TokenContainer>::const_iterator ItType;

    auto findSegmentByName = [](string segName, const vector<TokenSegment> &tokenSegments) -> vector<TokenSegment>::const_iterator {
        for (auto it = tokenSegments.begin(); it != tokenSegments.end(); ++it)
            if (it->segName == segName)
                return it;

        return tokenSegments.end();
    };

    auto locateMemoryBrackets = [](ItType it, ItType endIt) -> ItType {
        while ((it != endIt) &&
               (it->token.type() == Token::Type::MEMORY_BRACKET) &&
               (it->token.value<Token::MemoryBracket>() == Token::MemoryBracket::OPEN))
        {
            ItType bracketIt = it;

            while ((it != endIt) &&
                   ((it->token.type() != Token::Type::MEMORY_BRACKET) ||
                    (it->token.value<Token::MemoryBracket>() != Token::MemoryBracket::CLOSE)))
            {
                ++it;
            }

            if (it == endIt)
                throw CompileError("unclosed bracket", bracketIt->pos);

            ++it;
        }

        return it;
    };

    function<ItType(ItType, ItType)> locateInstructionOperand = [&](ItType begin, ItType end) -> ItType {
        auto it = begin;

        if (it == end)
            return it;

        if ((it->token.type() == Token::Type::REGISTER) &&
            ((it->token.value<Token::Register>().match(UREG)) ||
             ((it->token.value<Token::Register>().match(SREG)) &&
              (((it + 1) == end) ||
               ((it + 1)->token.type() != Token::Type::COLON)))))
        {
            return it + 1;
        }

        if (it->token.type() == Token::Type::SIZE_IDENTIFIER) {
            ++it;
            if (it->token.type() == Token::Type::SIZE_OPERATOR)
                ++it;
        }

        if ((it != end) &&
            (it->token.type() == Token::Type::REGISTER) &&
            (it->token.value<Token::Register>().match(SREG)) &&
            ((it + 1) != end) &&
            ((it + 1)->token.type() == Token::Type::COLON))
        {
            it += 2;
        }

        auto operandEndIt = locateMemoryBrackets(getMathTokenSequence(it, end), end);

        if ((operandEndIt == it) && (it != begin))
            throw CompileError("incomplete operand", (it - 1)->pos);

        return operandEndIt;
    };

    function<ItType(ItType, ItType)> locateDataOperand = [&](ItType begin, ItType end) -> ItType {
        auto it = begin;

        if (it == end)
            return it;

        if ((it->token.type() == Token::Type::CONSTANT_NUMBER) ||
            (it->token.type() == Token::Type::MATH_SYMBOL) ||
            (it->token.type() == Token::Type::USER_IDENTIFIER))
        {
            return getMathTokenSequence(it, end);
        }

        if (it->token.type() == Token::Type::CONSTANT_STRING)
            return it + 1;

        return it;
    };

    vector<PseudoSentencesSegment> segmentPseudoSentenceVector;
    map<string, Label> labelMap;

    for (auto segIt = segmentTokenContainerVector.begin(); segIt != segmentTokenContainerVector.end(); ++segIt) {
        vector<PseudoSentence> pseudoSentenceVector;

        Assume currentAssume;

        auto beginIt = segIt->tokenContainers.begin();
        auto endIt = segIt->tokenContainers.end();
        auto it = beginIt;
        while (it != endIt) {
            if (it->token.type() == Token::Type::USER_IDENTIFIER) {
                ++it;
                if (it == endIt)
                    throw CompileError("must be colon or size identifier", it->pos);

                if (it->token.type() == Token::Type::COLON) {
                    if (labelMap.count((it - 1)->token.value<string>()))
                        throw CompileError("duplicate label", (it - 1)->pos);

                    labelMap[(it - 1)->token.value<string>()] = {nullopt,
                                                                 pseudoSentenceVector.size(),
                                                                 segIt->segName};
                    
                    ++it;
                } else if (it->token.type() == Token::Type::DATA_IDENTIFIER) {
                    if (labelMap.count((it - 1)->token.value<string>()))
                        throw CompileError("duplicate label", (it - 1)->pos);

                    labelMap[(it - 1)->token.value<string>()] = {it->token.value<Token::DataIdentifier>(),
                                                                 pseudoSentenceVector.size(),
                                                                 segIt->segName};
                } else
                    throw CompileError("must be colon or size identifier", (it - 1)->pos);
            } else if ((it->token.type() == Token::Type::INSTRUCTION) ||
                       (it->token.type() == Token::Type::DATA_IDENTIFIER))
            {
                PseudoSentence pseudoSentence;
                pseudoSentence.baseTokenContainer = *it;
                function<ItType(ItType, ItType)> locateOperand = (it->token.type() == Token::Type::INSTRUCTION) ? locateInstructionOperand : locateDataOperand;
                ++it;

                auto operandEndIt = locateOperand(it, endIt);
                if (operandEndIt != it) {
                    pseudoSentence.operandsTokenContainerVector.push_back(vector<TokenContainer>(it, operandEndIt));
                    it = operandEndIt;

                    while ((it != endIt) && (it->token.type() == Token::Type::COMMA)) {
                        ++it;
                        if ((it == endIt) || (locateOperand(it, endIt) == it))
                            throw CompileError("no operand after comma", (it - 1)->pos);

                        auto operandEndIt = locateOperand(it, endIt);
                        pseudoSentence.operandsTokenContainerVector.push_back(vector<TokenContainer>(it, operandEndIt));
                        it = operandEndIt;
                    }
                }

                if ((it != endIt) &&
                    (pseudoSentence.operandsTokenContainerVector.size() == 1) &&
                    (pseudoSentence.operandsTokenContainerVector[0].size() == 1) &&
                    (pseudoSentence.operandsTokenContainerVector[0][0].token.type() == Token::Type::USER_IDENTIFIER) &&
                    ((it->token.type() == Token::Type::COLON) ||
                     ((it->token.type() == Token::Type::DATA_IDENTIFIER) &&
                      (pseudoSentence.operandsTokenContainerVector[0][0].pos.row != pseudoSentence.baseTokenContainer.pos.row))))
                {
                    pseudoSentence.operandsTokenContainerVector.clear();
                    --it;
                }

                pseudoSentence.assume = currentAssume;
                pseudoSentenceVector.push_back(pseudoSentence);
            } else if (it->token.type() == Token::Type::ASSUME_DIRECTIVE) {
                ++it;
                while (true) {
                    if ((it == segIt->tokenContainers.end()) ||
                        ((it + 1) == segIt->tokenContainers.end()) ||
                        ((it + 2) == segIt->tokenContainers.end()))
                    {
                        throw CompileError("unexpected ASSUME end", (segIt->tokenContainers.end() - 1)->pos);
                    }

                    if ((it->token.type() != Token::Type::REGISTER) ||
                        (!it->token.value<Token::Register>().match(OperandMask::SREG)))
                    {
                        throw CompileError("must be segment register here", it->pos);
                    }

                    if ((it + 1)->token.type() != Token::Type::COLON)
                        throw CompileError("must be colon here", (it + 1)->pos);

                    if ((it + 2)->token.type() != Token::Type::USER_IDENTIFIER)
                        throw CompileError("must be segment name here", (it + 2)->pos);

                    auto currentSegIt = findSegmentByName((it + 2)->token.value<string>(), segmentTokenContainerVector);

                    if (currentSegIt == segmentTokenContainerVector.end())
                        throw CompileError("must be segment name here", (it + 2)->pos);

                    currentAssume.setSegment(currentSegIt->segName, it->token.value<Token::Register>());

                    if (((it + 3) == segIt->tokenContainers.end()) || ((it + 3)->token.type() != Token::Type::COMMA)) {
                        it += 3;
                        break;
                    }

                    it += 4;
                }
            } else
                throw CompileError("an instruction, data identifier, assume or label expected", it->pos);
        }

        segmentPseudoSentenceVector.push_back({segIt->segName, pseudoSentenceVector});
    }

    return make_tuple(segmentPseudoSentenceVector, labelMap);
}
