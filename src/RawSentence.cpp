#include "RawSentence.h"

#include "Compiler.h"
#include "Preprocessor.h"
#include "Math.h"
#include "Exception.h"
#include "Diagnostics.h"
#include "Instruction.h"
#include <utility>
#include <algorithm>

using namespace OperandMask;

RawInstructionSentence::SegmentPrefix getSegmentOverridePrefix(Token::Register reg) {
    if (reg.match(ES))
        return RawInstructionSentence::SegmentPrefix::ES;
    else if (reg.match(CS))
        return RawInstructionSentence::SegmentPrefix::CS;
    else if (reg.match(SS))
        return RawInstructionSentence::SegmentPrefix::SS;
    else if (reg.match(DS))
        return RawInstructionSentence::SegmentPrefix::DS;
    else if (reg.match(FS))
        return RawInstructionSentence::SegmentPrefix::FS;
    else
        return RawInstructionSentence::SegmentPrefix::GS;
}

void pureMathThrowableCheck(vector<TokenContainer>::const_iterator begin, vector<TokenContainer>::const_iterator end) {
    for (auto it = begin; it != end; ++it) {
        if ((it->token.type() != Token::Type::MATH_SYMBOL) &&
            (it->token.type() != Token::Type::CONSTANT_NUMBER))
        {
            throw CompileError("unexpected token", it->pos);
        }
    }
};

auto convertToMathOperationVector(const vector<TokenContainer> &tokenContainerVector) {
    vector<MathOperation> mathOperationVector;

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it) {
        const TokenContainer &tokenContainer = *it;
        const Token &token = tokenContainer.token;

        MathOperation currentOperation;

        if (token.type() == Token::Type::MATH_SYMBOL) {
            MathOperationKind currentOperationKind;
            switch (token.value<Token::MathSymbol>()) {
            case Token::MathSymbol::PLUS:
                currentOperationKind = MathOperationKind::ADD;
                break;
            case Token::MathSymbol::MINUS:
                currentOperationKind = MathOperationKind::SUBTRACT;
                break;
            case Token::MathSymbol::MULTIPLY:
                currentOperationKind = MathOperationKind::MULTIPLY;
                break;
            case Token::MathSymbol::DIVIDE:
                currentOperationKind = MathOperationKind::DIVIDE;
                break;
            case Token::MathSymbol::BRACKET_OPEN:
                currentOperationKind = MathOperationKind::BRACKET_OPEN;
                break;
            case Token::MathSymbol::BRACKET_CLOSE:
                currentOperationKind = MathOperationKind::BRACKET_CLOSE;
                break;
            }

            currentOperation = {currentOperationKind,
                                0,
                                tokenContainer.pos};
        } else if (token.type() == Token::Type::CONSTANT_NUMBER) {
            currentOperation = {MathOperationKind::CONSTANT,
                                token.value<Integer>(),
                                tokenContainer.pos};
        }

        mathOperationVector.push_back(currentOperation);
    }

    return mathOperationVector;
};

RawInstructionSentence::RawInstructionSentence(const PseudoSentence &pseudoSentence, const map<string, Label> &labelMap) :
    RawSentence(pseudoSentence.baseTokenContainer.pos, pseudoSentence.assume),
    instruction(pseudoSentence.baseTokenContainer.token.value<Token::Instruction>())
{
    typedef vector<TokenContainer>::const_iterator ItType;

    auto locateMemoryBracketExpression = [](ItType it, ItType endIt) -> ItType {
        if ((it == endIt) ||
            (it->token.type() != Token::Type::MEMORY_BRACKET) ||
            (it->token.value<Token::MemoryBracket>() != Token::MemoryBracket::OPEN))
        {
            return it;
        }

        while ((it->token.type() != Token::Type::MEMORY_BRACKET) ||
               (it->token.value<Token::MemoryBracket>() != Token::MemoryBracket::CLOSE))
        {
            ++it;
        }

        return it + 1;
    };

    auto isPureMath = [](ItType it, ItType endIt) -> bool {
        while (it != endIt) {
            if ((it->token.type() != Token::Type::MATH_SYMBOL) &&
                (it->token.type() != Token::Type::CONSTANT_NUMBER))
            {
                return false;
            }

            ++it;
        }

        return true;
    };

    const vector<vector<TokenContainer>> &operandsTokenContainerVector = pseudoSentence.operandsTokenContainerVector;

    for (auto it = operandsTokenContainerVector.begin(); it != operandsTokenContainerVector.end(); ++it) {
        const vector<TokenContainer> &tokenContainerVector = *it;

        if (tokenContainerVector.size() == 1) {
            const Token &token = tokenContainerVector[0].token;

            if (token.type() == Token::Type::REGISTER) {
                operandContainerVector.push_back(OperandContainer({token.value<Token::Register>(), {0, nullopt, false}}, tokenContainerVector[0].pos));
                continue;
            }
        }

        CodePosition operandPos = calculatePos(tokenContainerVector.begin(), tokenContainerVector.end());

        auto jt = tokenContainerVector.begin();
        
        Mask opMask;
        
        if (jt->token.type() == Token::Type::SIZE_IDENTIFIER) {
            switch (jt->token.value<Token::SizeIdentifier>()) {
            case Token::SizeIdentifier::BYTE:
                opMask |= S8;
                break;
            case Token::SizeIdentifier::WORD:
                opMask |= S16;
                break;
            case Token::SizeIdentifier::DWORD:
                opMask |= S32;
                break;
            }
            ++jt;

            if (jt->token.type() == Token::Type::SIZE_OPERATOR)
                ++jt;
        }

        bool thisOpPrefixOverride = false;
        if ((jt->token.type() == Token::Type::REGISTER) &&
            (jt->token.value<Token::Register>().match(SREG)))
        {
            segmentPrefix = getSegmentOverridePrefix(jt->token.value<Token::Register>());
            thisOpPrefixOverride = true;

            jt += 2;
        }

        vector<vector<TokenContainer>> rawOperandTokenContainerVectors;

        auto firstEndIt = getMathTokenSequence(jt, tokenContainerVector.end());
        if ((firstEndIt == tokenContainerVector.end()) &&
            isPureMath(jt, firstEndIt))
        {
            if (thisOpPrefixOverride)
                throw CompileError("constant cannot have segment override", operandPos);

            Integer constant = mathExpressionComputer(convertToMathOperationVector({jt, firstEndIt}));
            operandContainerVector.push_back(OperandContainer({opMask | IMM, {constant, nullopt, false}}, operandPos));
            continue;
        }
        rawOperandTokenContainerVectors.push_back({jt, firstEndIt});
        jt = firstEndIt;

        while (jt != tokenContainerVector.end()) {
            auto bracketEndIt = locateMemoryBracketExpression(jt, tokenContainerVector.end());
            rawOperandTokenContainerVectors.push_back({jt + 1, bracketEndIt - 1});
            jt = bracketEndIt;
        }

        vector<vector<TokenContainer>> operandTokenContainerVectors;

        for (auto kt = rawOperandTokenContainerVectors.begin(); kt != rawOperandTokenContainerVectors.end(); ++kt) {
            auto firstIt = kt->begin();
            
            for (auto lt = kt->begin(); lt != kt->end(); ++lt) {
                if ((lt->token.type() == Token::Type::MATH_SYMBOL) &&
                    (lt->token.value<Token::MathSymbol>() == Token::MathSymbol::PLUS))
                {
                    operandTokenContainerVectors.push_back({firstIt, lt});
                    firstIt = lt + 1;

                    if (firstIt == kt->end())
                        throw CompileError("'+' must have right value", lt->pos);
                }
            }

            operandTokenContainerVectors.push_back({firstIt, kt->end()});
        }

        RawNumber disp = {0, nullopt, false};
        
        Mask baseReg;
        Mask indexReg;
        Mask indexMult;
        for (auto kt = operandTokenContainerVectors.begin(); kt != operandTokenContainerVectors.end(); ++kt) {
            if (kt->size() == 1) {
                TokenContainer &tokenContainer = (*kt)[0];
                Token &token = tokenContainer.token;

                if (token.type() == Token::Type::REGISTER) {
                    Token::Register reg = token.value<Token::Register>();

                    if (reg.match(UREG16)) {
                        if (!opMask.matchAny(MEM_MODE_ANY))
                            opMask |= MEM_16;
                        
                        if (opMask.match(MEM_32))
                            throw CompileError("incorrect size of register", tokenContainer.pos);

                        if (!baseReg.matchAny(UREG_ANY)) {
                            baseReg = reg;
                            continue;
                        } else if (!indexReg.matchAny(UREG_ANY)) {
                            indexReg = reg;
                            continue;
                        }

                        throw CompileError("unexpected token", tokenContainer.pos);
                    } else if (reg.match(UREG32)) {
                        if (!opMask.matchAny(MEM_MODE_ANY))
                            opMask |= MEM_32;

                        if (opMask.match(MEM_16))
                            throw CompileError("incorrect size of register", tokenContainer.pos);

                        if (!baseReg.matchAny(UREG_ANY)) {
                            baseReg = reg;
                            continue;
                        } else if (!indexReg.matchAny(UREG_ANY)) {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT1;
                            continue;
                        }

                        throw CompileError("unexpected token", tokenContainer.pos);
                    }
                }
            }

            if ((kt->size() == 3) &&
                ((*kt)[1].token.type() == Token::Type::MATH_SYMBOL) &&
                ((*kt)[1].token.value<Token::MathSymbol>() == Token::MathSymbol::MULTIPLY))
            {
                if ((((*kt)[0].token.type() == Token::Type::REGISTER) &&
                     ((*kt)[0].token.value<Token::Register>().match(UREG32)) &&
                     ((*kt)[2].token.type() == Token::Type::CONSTANT_NUMBER)) ||
                    (((*kt)[2].token.type() == Token::Type::REGISTER) &&
                     ((*kt)[0].token.value<Token::Register>().match(UREG32)) &&
                     ((*kt)[0].token.type() == Token::Type::CONSTANT_NUMBER)))
                {
                    TokenContainer regTokenContainer;
                    TokenContainer immTokenContainer;

                    if ((*kt)[0].token.type() == Token::Type::REGISTER) {
                        regTokenContainer = (*kt)[0];
                        immTokenContainer = (*kt)[2];
                    } else {
                        regTokenContainer = (*kt)[2];
                        immTokenContainer = (*kt)[0];
                    }

                    if (!opMask.matchAny(MEM_MODE_ANY))
                        opMask |= MEM_32;

                    if (opMask.match(MEM_16))
                        throw CompileError("incorrect size of register", regTokenContainer.pos);

                    if (indexReg.matchAny(UREG_ANY))
                        throw CompileError("unexpected token", calculatePos(kt->begin(), kt->end()));
                    else {
                        Integer indexIntFact = immTokenContainer.token.value<Integer>();
                        Mask reg = regTokenContainer.token.value<Mask>();
                        
                        if (indexIntFact == 1) {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT1;
                            continue;
                        } else if (indexIntFact == 2) {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT2;
                            continue;
                        } else if (indexIntFact == 4) {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT4;
                            continue;
                        } else if (indexIntFact == 8) {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT8;
                            continue;
                        } else
                            throw CompileError("unsupported factor", immTokenContainer.pos);
                    }
                }
            }

            if (!kt->empty()) {
                auto lt = kt->begin();
                
                if (lt->token.type() == Token::Type::USER_IDENTIFIER) {
                    if (disp.label)
                        throw CompileError("you can use only one pointer in addressing", lt->pos);

                    auto labelIt = labelMap.find(lt->token.value<string>());
                    if (labelIt == labelMap.end())
                        throw CompileError("undefined label", lt->pos);

                    disp.label = labelIt->second;
                    disp.isNotFinal = true;

                    ++lt;
                }

                if (lt != kt->end()) {
                    pureMathThrowableCheck(lt, kt->end());

                    disp.num += mathExpressionComputer(convertToMathOperationVector({lt, kt->end()}));
                }
            }
        }

        if (!opMask.matchAny(MEM_MODE_ANY)) {
            //TODO: Implement check by Integer size
            switch (Compiler::arch) {
            case Compiler::Arch::X86_16:
                opMask |= MEM_16;
                break;
            case Compiler::Arch::X86_32:
                opMask |= MEM_32;
                break;
            }
        }

        if (opMask.match(MEM_16)) {
            if (baseReg.matchAny(UREG_ANY)) {
                if (baseReg.match(AX) ||
                    baseReg.match(CX) ||
                    baseReg.match(DX) ||
                    baseReg.match(SP))
                {
                    throw CompileError("in 16 bit addressing mode registers can only be bx,bp,si,di", operandPos);
                }

                if (indexReg.matchAny(UREG_ANY)) {
                    if (indexReg.match(AX) ||
                        indexReg.match(CX) ||
                        indexReg.match(DX) ||
                        indexReg.match(SP))
                    {
                        throw CompileError("in 16 bit addressing mode registers can only be bx,bp,si,di", operandPos);
                    }

                    if (((baseReg.match(BX) || baseReg.match(BP)) &&
                         (indexReg.match(BX) || indexReg.match(BP))) ||
                        ((baseReg.match(SI) || baseReg.match(DI)) &&
                         (indexReg.match(SI) || indexReg.match(DI))))
                    {
                        throw CompileError("in 16 bit addressing mode base register can only be bx or bp, and index si or di", operandPos);
                    }
                }
            }
        } else {
            if (indexReg.match(ESP)) {
                if (!indexMult.match(MEM_32_INDEX_MULT1))
                    throw CompileError("you cannot use esp as index register", operandPos);

                if (baseReg.matchAny(UREG_ANY)) {
                    if (baseReg.match(ESP))
                        throw CompileError("you cannot use esp as index register", operandPos);

                    std::swap(baseReg, indexReg);
                } else {
                    baseReg = indexReg;
                    indexReg.reset();
                    indexMult.reset();
                }
            }
        }
        
        if (opMask.match(MEM_16)) {
            if (baseReg.match(BX)) {
                if (indexReg.match(SI))
                    opMask |= MEM_16_BX_SI;
                else if (indexReg.match(DI))
                    opMask |= MEM_16_BX_DI;
                else
                    opMask |= MEM_16_BX;
            } else if (baseReg.match(BP)) {
                if (indexReg.match(SI))
                    opMask |= MEM_16_BP_SI;
                else if (indexReg.match(DI))
                    opMask |= MEM_16_BP_DI;
                else
                    opMask |= MEM_16_BP;
            } else if (baseReg.match(SI))
                opMask |= MEM_16_SI;
            else if (baseReg.match(DI))
                opMask |= MEM_16_DI;
        } else {
            Mask baseRegPureData = baseReg & OPVAL_ANY;
            Mask indexRegPureData = indexReg & OPVAL_ANY;

            if (baseRegPureData.any())
                opMask |= MEM_32 | MEM_BASE | baseRegPureData;

            if (indexRegPureData.any())
                opMask |= indexMult | (indexRegPureData << 8);
        }

        operandContainerVector.push_back(OperandContainer({opMask, disp}, operandPos));
    }

    if (InstructionNS::jumpInstructionsSet.count(instruction) &&
        (operandContainerVector.size() == 1))
    {
        Operand &op = get<0>(operandContainerVector[0]);

        if (op.mask.match(IMM) ||
            Mask(MEM_MODE_ANY).match(op.mask))
        {
            op.mask = REL;
        }
    }

    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it) {
        Operand &op = get<0>(*it);
        if (op.rawNum.label && op.mask.match(MEM) && (!op.mask.matchAny(S_ANY)) && op.rawNum.label->dataIdentifier) {
            switch (*op.rawNum.label->dataIdentifier) {
            case Token::DataIdentifier::DB:
                op.mask |= S8;
                break;
            case Token::DataIdentifier::DW:
                op.mask |= S16;
                break;
            case Token::DataIdentifier::DD:
                op.mask |= S32;
                break;
            }
        }
    }
}

tuple<string, vector<string>> RawInstructionSentence::present(const map<string, Label> &labelMap) const {
    string instructionStr = findByValue(InstructionNS::instructionMap, instruction)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
        operandStrVector.push_back(get<0>(*it).present(labelMap));

    return make_tuple(instructionStr, operandStrVector);
}

string RawInstructionSentence::Operand::present(const map<string, Label> &labelMap) const {
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

        optional<string> labelStr;
        if (rawNum.label) {
            auto labelIt = std::find_if(labelMap.begin(), labelMap.end(), LabelFinder(*rawNum.label));
            labelStr = labelIt->first;
        }

        if (innerMemStr.empty()) {
            if (rawNum.num == 0)
                if (labelStr)
                    innerMemStr += *labelStr;
                else
                    innerMemStr += "0";
            else {
                innerMemStr += rawNum.num.str();

                if (labelStr)
                    innerMemStr += "+" + *labelStr;
            }
        } else {
            if (rawNum.num == 0) {
                if (labelStr)
                    innerMemStr += "+" + *labelStr;
            } else {
                innerMemStr += rawNum.num.str(true);

                if (labelStr)
                    innerMemStr += "+" + *labelStr;
            }
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

        immStr += rawNum.num.str();

        return immStr;
    } else {
        optional<string> labelStr;
        if (rawNum.label) {
            auto labelIt = std::find_if(labelMap.begin(), labelMap.end(), LabelFinder(*rawNum.label));
            labelStr = labelIt->first;
        }

        string relStr;

        if (labelStr) {
            relStr += *labelStr;
            if (rawNum.num != 0)
                relStr += rawNum.num.str(true);
        } else
            relStr += rawNum.num.str();

        return relStr;
    }
}

RawDataSentence::RawDataSentence(const PseudoSentence &pseudoSentence, const map<string, Label> &labelMap) :
    RawSentence(pseudoSentence.baseTokenContainer.pos, pseudoSentence.assume),
    dataIdentifier(pseudoSentence.baseTokenContainer.token.value<Token::DataIdentifier>())
{
    const vector<vector<TokenContainer>> &operandsTokenContainerVector = pseudoSentence.operandsTokenContainerVector;

    for (auto it = operandsTokenContainerVector.begin(); it != operandsTokenContainerVector.end(); ++it) {
        CodePosition operandPos = calculatePos(it->begin(), it->end());

        if ((it->size() == 1) &&
            ((*it)[0].token.type() == Token::Type::CONSTANT_STRING))
        {
            if (dataIdentifier != DataIdentifier::DB)
                throw CompileError("you can use string constant only with DB", operandPos);

            string str = (*it)[0].token.value<string>();

            for (size_t i = 0; i < str.size(); ++i) {
                CodePosition charPos = {operandPos.row, operandPos.column + i + 1, 1};
                operandContainerVector.push_back(OperandContainer({(UInt)str[i], nullopt, false}, charPos));
            }

            continue;
        }

        vector<vector<TokenContainer>> operandTokenContainerVectors;

        auto firstIt = it->begin();
        for (auto jt = it->begin(); jt != it->end(); ++jt) {
            if ((jt->token.type() == Token::Type::MATH_SYMBOL) &&
                (jt->token.value<Token::MathSymbol>() == Token::MathSymbol::PLUS))
            {
                operandTokenContainerVectors.push_back({firstIt, jt});
                firstIt = jt + 1;

                if (firstIt == it->end())
                    throw CompileError("'+' must have right value", jt->pos);
            }
        }
        operandTokenContainerVectors.push_back({firstIt, it->end()});

        RawNumber rawNum;

        for (auto jt = operandTokenContainerVectors.begin(); jt != operandTokenContainerVectors.end(); ++jt) {
            if (!jt->empty()) {
                auto kt = jt->begin();
                
                if (kt->token.type() == Token::Type::USER_IDENTIFIER) {
                    if (rawNum.label)
                        throw CompileError("you can use only one pointer in data", kt->pos);

                    auto labelIt = labelMap.find(kt->token.value<string>());
                    if (labelIt == labelMap.end())
                        throw CompileError("undefined label", kt->pos);

                    rawNum.label = labelIt->second;
                    rawNum.isNotFinal = true;

                    ++kt;
                }

                if (kt != jt->end()) {
                    pureMathThrowableCheck(kt, jt->end());

                    rawNum.num += mathExpressionComputer(convertToMathOperationVector({kt, jt->end()}));
                }
            }
        }

        operandContainerVector.push_back(OperandContainer(rawNum, operandPos));
    }
}

tuple<string, vector<string>> RawDataSentence::present(const map<string, Label> &labelMap) const {
    string instructionStr = findByValue(InstructionNS::dataIdentifierMap, dataIdentifier)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it) {
        const RawNumber &rawNum = get<0>(*it);
        
        optional<string> labelStr;
        if (rawNum.label) {
            auto labelIt = std::find_if(labelMap.begin(), labelMap.end(), LabelFinder(*rawNum.label));
            labelStr = labelIt->first;
        }

        string str;
        if (rawNum.num == 0) {
            if (labelStr)
                str += *labelStr;
            else
                str += "0";
        } else {
            str += rawNum.num.str();

            if (labelStr)
                str += "+" + *labelStr;
        }

        operandStrVector.push_back(str);
    }

    return make_tuple(instructionStr, operandStrVector);
}

vector<RawSentencesSegment> constructRawSentences(const vector<PseudoSentencesSegment> &pseudoSentencesSegmentContainerVector,
                                                    const map<string, Label> &labelMap)
{
    vector<RawSentencesSegment> rawSentencesSegmentContainerVector;

    for (auto it = pseudoSentencesSegmentContainerVector.begin(); it != pseudoSentencesSegmentContainerVector.end(); ++it) {
        vector<shared_ptr<RawSentence>> rawSentenceVector;

        for (auto jt = it->pseudoSentences.begin(); jt != it->pseudoSentences.end(); ++jt) {
            if (jt->baseTokenContainer.token.type() == Token::Type::INSTRUCTION)
                rawSentenceVector.push_back(shared_ptr<RawSentence>(new RawInstructionSentence(*jt, labelMap)));
            else
                rawSentenceVector.push_back(shared_ptr<RawSentence>(new RawDataSentence(*jt, labelMap)));
        }

        rawSentencesSegmentContainerVector.push_back({it->segName, rawSentenceVector});
    }

    return rawSentencesSegmentContainerVector;
}
