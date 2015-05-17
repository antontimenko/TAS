#include "TSRawSentence.h"

#include "TSCompiler.h"
#include "TSPreprocessor.h"
#include "TSMath.h"
#include "TSException.h"
#include "TSDiagnostics.h"
#include <utility>

using namespace TSOperandMask;

TSCodePosition TSRawSentence::pos() const
{
    return _pos;
}

TSRawInstructionSentence::Prefix getSegmentOverridePrefix(const TSToken &token)
{
    TSToken::Register reg = token.value<TSToken::Register>();

    if (reg.match(ES))
        return TSRawInstructionSentence::Prefix::ES;
    else if (reg.match(CS))
        return TSRawInstructionSentence::Prefix::CS;
    else if (reg.match(SS))
        return TSRawInstructionSentence::Prefix::SS;
    else if (reg.match(DS))
        return TSRawInstructionSentence::Prefix::DS;
    else if (reg.match(FS))
        return TSRawInstructionSentence::Prefix::FS;
    else
        return TSRawInstructionSentence::Prefix::GS;
}

void pureMathThrowableCheck(vector<TSTokenContainer>::const_iterator begin, vector<TSTokenContainer>::const_iterator end)
{
    for (auto it = begin; it != end; ++it)
    {
        if ((it->token.type() != TSToken::Type::MATH_SYMBOL) &&
            (it->token.type() != TSToken::Type::CONSTANT_NUMBER))
        {
            throw TSCompileError("unexpected token", it->pos);
        }
    }
};

auto convertToMathOperationVector(const vector<TSTokenContainer> &tokenContainerVector)
{
    vector<TSMathOperation> mathOperationVector;

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        const TSTokenContainer &tokenContainer = *it;
        const TSToken &token = tokenContainer.token;

        TSMathOperation currentOperation;

        if (token.type() == TSToken::Type::MATH_SYMBOL)
        {
            TSMathOperationKind currentOperationKind;
            switch (token.value<TSToken::MathSymbol>())
            {
            case TSToken::MathSymbol::PLUS:
                currentOperationKind = TSMathOperationKind::ADD;
                break;
            case TSToken::MathSymbol::MINUS:
                currentOperationKind = TSMathOperationKind::SUBTRACT;
                break;
            case TSToken::MathSymbol::MULTIPLY:
                currentOperationKind = TSMathOperationKind::MULTIPLY;
                break;
            case TSToken::MathSymbol::DIVIDE:
                currentOperationKind = TSMathOperationKind::DIVIDE;
                break;
            case TSToken::MathSymbol::BRACKET_OPEN:
                currentOperationKind = TSMathOperationKind::BRACKET_OPEN;
                break;
            case TSToken::MathSymbol::BRACKET_CLOSE:
                currentOperationKind = TSMathOperationKind::BRACKET_CLOSE;
                break;
            }

            currentOperation = {currentOperationKind,
                                0,
                                tokenContainer.pos};
        }
        else if (token.type() == TSToken::Type::CONSTANT_NUMBER)
        {
            currentOperation = {TSMathOperationKind::CONSTANT,
                                token.value<TSInteger>(),
                                tokenContainer.pos};
        }

        mathOperationVector.push_back(currentOperation);
    }

    return mathOperationVector;
};

TSRawInstructionSentence::TSRawInstructionSentence(const TSPseudoSentence &pseudoSentence) :
    TSRawSentence(pseudoSentence.baseTokenContainer.pos),
    instruction(pseudoSentence.baseTokenContainer.token.value<TSToken::Instruction>())
{
    typedef vector<TSTokenContainer>::const_iterator ItType;

    auto locateMemoryBracketExpression = [](ItType it, ItType endIt) -> ItType {
        if ((it == endIt) ||
            (it->token.type() != TSToken::Type::MEMORY_BRACKET) ||
            (it->token.value<TSToken::MemoryBracket>() != TSToken::MemoryBracket::OPEN))
        {
            return it;
        }

        while ((it->token.type() != TSToken::Type::MEMORY_BRACKET) ||
               (it->token.value<TSToken::MemoryBracket>() != TSToken::MemoryBracket::CLOSE))
        {
            ++it;
        }

        return it + 1;
    };

    auto isPureMath = [](ItType it, ItType endIt) -> bool {
        while (it != endIt)
        {
            if ((it->token.type() != TSToken::Type::MATH_SYMBOL) &&
                (it->token.type() != TSToken::Type::CONSTANT_NUMBER))
            {
                return false;
            }

            ++it;
        }

        return true;
    };

    const vector<vector<TSTokenContainer>> &operandsTokenContainerVector = pseudoSentence.operandsTokenContainerVector;

    for (auto it = operandsTokenContainerVector.begin(); it != operandsTokenContainerVector.end(); ++it)
    {
        const vector<TSTokenContainer> &tokenContainerVector = *it;

        if (tokenContainerVector.size() == 1)
        {
            const TSToken &token = tokenContainerVector[0].token;

            if (token.type() == TSToken::Type::REGISTER)
            {
                operandContainerVector.push_back(OperandContainer({token.value<TSToken::Register>(), {0, nullopt}}, tokenContainerVector[0].pos));
                continue;
            }
        }

        TSCodePosition operandPos = calculatePos(tokenContainerVector.begin(), tokenContainerVector.end());

        auto jt = tokenContainerVector.begin();
        
        Mask opMask;
        
        if (jt->token.type() == TSToken::Type::SIZE_IDENTIFIER)
        {
            switch (jt->token.value<TSToken::SizeIdentifier>())
            {
            case TSToken::SizeIdentifier::BYTE:
                opMask |= S8;
                break;
            case TSToken::SizeIdentifier::WORD:
                opMask |= S16;
                break;
            case TSToken::SizeIdentifier::DWORD:
                opMask |= S32;
                break;
            }
            ++jt;

            if (jt->token.type() == TSToken::Type::SIZE_OPERATOR)
                ++jt;
        }

        if ((jt->token.type() == TSToken::Type::REGISTER) &&
            (jt->token.value<TSToken::Register>().match(SREG)))
        {
            prefixVector.push_back(getSegmentOverridePrefix(jt->token));

            jt += 2;
        }

        vector<vector<TSTokenContainer>> rawOperandTokenContainerVectors;

        auto firstEndIt = getMathTokenSequence(jt, tokenContainerVector.end());
        if ((firstEndIt == tokenContainerVector.end()) &&
            isPureMath(jt, firstEndIt))
        {
            if (!prefixVector.empty())
                throw TSCompileError("constant cannot have segment override", operandPos);

            TSInteger constant = mathExpressionComputer(convertToMathOperationVector({jt, firstEndIt}));
            TSRawNumber rawNum{constant, nullopt};
            operandContainerVector.push_back(OperandContainer({opMask | IMM, rawNum}, operandPos));
            continue;
        }
        rawOperandTokenContainerVectors.push_back({jt, firstEndIt});
        jt = firstEndIt;

        while (jt != tokenContainerVector.end())
        {
            auto bracketEndIt = locateMemoryBracketExpression(jt, tokenContainerVector.end());
            rawOperandTokenContainerVectors.push_back({jt + 1, bracketEndIt - 1});
            jt = bracketEndIt;
        }

        vector<vector<TSTokenContainer>> operandTokenContainerVectors;

        for (auto kt = rawOperandTokenContainerVectors.begin(); kt != rawOperandTokenContainerVectors.end(); ++kt)
        {
            auto firstIt = kt->begin();
            
            for (auto lt = kt->begin(); lt != kt->end(); ++lt)
            {
                if ((lt->token.type() == TSToken::Type::MATH_SYMBOL) &&
                    (lt->token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::PLUS))
                {
                    operandTokenContainerVectors.push_back({firstIt, lt});
                    firstIt = lt + 1;

                    if (firstIt == kt->end())
                        throw TSCompileError("'+' must have right value", lt->pos);
                }
            }

            operandTokenContainerVectors.push_back({firstIt, kt->end()});
        }

        TSRawNumber disp;
        
        Mask baseReg;
        Mask indexReg;
        Mask indexMult;
        for (auto kt = operandTokenContainerVectors.begin(); kt != operandTokenContainerVectors.end(); ++kt)
        {
            if (kt->size() == 1)
            {
                TSTokenContainer &tokenContainer = (*kt)[0];
                TSToken &token = tokenContainer.token;

                if (token.type() == TSToken::Type::REGISTER)
                {
                    TSToken::Register reg = token.value<TSToken::Register>();

                    if (reg.match(UREG16))
                    {
                        if (!opMask.matchAny(MEM_MODE_ANY))
                            opMask |= MEM_16;
                        
                        if (opMask.match(MEM_32))
                            throw TSCompileError("incorrect size of register", tokenContainer.pos);

                        if (!baseReg.matchAny(UREG_ANY))
                        {
                            baseReg = reg;
                            continue;
                        }
                        else if (!indexReg.matchAny(UREG_ANY))
                        {
                            indexReg = reg;
                            continue;
                        }

                        throw TSCompileError("unexpected token", tokenContainer.pos);
                    }
                    else if (reg.match(UREG32))
                    {
                        if (!opMask.matchAny(MEM_MODE_ANY))
                            opMask |= MEM_32;

                        if (opMask.match(MEM_16))
                            throw TSCompileError("incorrect size of register", tokenContainer.pos);

                        if (!baseReg.matchAny(UREG_ANY))
                        {
                            baseReg = reg;
                            continue;
                        }
                        else if (!indexReg.matchAny(UREG_ANY))
                        {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT1;
                            continue;
                        }

                        throw TSCompileError("unexpected token", tokenContainer.pos);
                    }
                }
            }

            if ((kt->size() == 3) &&
                ((*kt)[1].token.type() == TSToken::Type::MATH_SYMBOL) &&
                ((*kt)[1].token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::MULTIPLY))
            {
                if ((((*kt)[0].token.type() == TSToken::Type::REGISTER) &&
                     ((*kt)[0].token.value<TSToken::Register>().match(UREG32)) &&
                     ((*kt)[2].token.type() == TSToken::Type::CONSTANT_NUMBER)) ||
                    (((*kt)[2].token.type() == TSToken::Type::REGISTER) &&
                     ((*kt)[0].token.value<TSToken::Register>().match(UREG32)) &&
                     ((*kt)[0].token.type() == TSToken::Type::CONSTANT_NUMBER)))
                {
                    TSTokenContainer regTokenContainer;
                    TSTokenContainer immTokenContainer;

                    if ((*kt)[0].token.type() == TSToken::Type::REGISTER)
                    {
                        regTokenContainer = (*kt)[0];
                        immTokenContainer = (*kt)[2];
                    }
                    else
                    {
                        regTokenContainer = (*kt)[2];
                        immTokenContainer = (*kt)[0];
                    }

                    if (!opMask.matchAny(MEM_MODE_ANY))
                        opMask |= MEM_32;

                    if (opMask.match(MEM_16))
                        throw TSCompileError("incorrect size of register", regTokenContainer.pos);

                    if (indexReg.matchAny(UREG_ANY))
                        throw TSCompileError("unexpected token", calculatePos(kt->begin(), kt->end()));
                    else
                    {
                        TSInteger indexIntFact = immTokenContainer.token.value<TSInteger>();
                        Mask reg = regTokenContainer.token.value<Mask>();
                        
                        if (indexIntFact == 1)
                        {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT1;
                            continue;
                        }
                        else if (indexIntFact == 2)
                        {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT2;
                            continue;
                        }
                        else if (indexIntFact == 4)
                        {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT4;
                            continue;
                        }
                        else if (indexIntFact == 8)
                        {
                            indexReg = reg;
                            indexMult = MEM_32_INDEX_MULT8;
                            continue;
                        }
                        else
                            throw TSCompileError("unsupported factor", immTokenContainer.pos);
                    }
                }
            }

            if (!kt->empty())
            {
                auto lt = kt->begin();
                
                if (lt->token.type() == TSToken::Type::USER_IDENTIFIER)
                {
                    if (disp.labelStr)
                        throw TSCompileError("you can use only one pointer in addressing", lt->pos);

                    disp.labelStr = lt->token.value<string>();

                    ++lt;
                }

                if (lt != kt->end())
                {
                    pureMathThrowableCheck(lt, kt->end());

                    disp.num += mathExpressionComputer(convertToMathOperationVector({lt, kt->end()}));
                }
            }
        }

        if (!opMask.matchAny(MEM_MODE_ANY))
        {
            //TODO: Implement check by TSInteger size
            switch (TSCompiler::defaultArch)
            {
            case TSCompiler::Arch::X86_16:
                opMask |= MEM_16;
                break;
            case TSCompiler::Arch::X86_32:
                opMask |= MEM_32;
                break;
            }
        }

        if (opMask.match(MEM_16))
        {
            if (baseReg.matchAny(UREG_ANY))
            {
                if (baseReg.match(AX) ||
                    baseReg.match(CX) ||
                    baseReg.match(DX) ||
                    baseReg.match(SP))
                {
                    throw TSCompileError("in 16 bit addressing mode registers can only be bx,bp,si,di", operandPos);
                }

                if (indexReg.matchAny(UREG_ANY))
                {
                    if (indexReg.match(AX) ||
                        indexReg.match(CX) ||
                        indexReg.match(DX) ||
                        indexReg.match(SP))
                    {
                        throw TSCompileError("in 16 bit addressing mode registers can only be bx,bp,si,di", operandPos);
                    }

                    if (((baseReg.match(BX) || baseReg.match(BP)) &&
                         (indexReg.match(BX) || indexReg.match(BP))) ||
                        ((baseReg.match(SI) || baseReg.match(DI)) &&
                         (indexReg.match(SI) || indexReg.match(DI))))
                    {
                        throw TSCompileError("in 16 bit addressing mode base register can only be bx or bp, and index si or di", operandPos);
                    }
                }
            }
        }
        else
        {
            if (indexReg.match(ESP))
            {
                if (!indexMult.match(MEM_32_INDEX_MULT1))
                    throw TSCompileError("you cannot use esp as index register", operandPos);

                if (baseReg.matchAny(UREG_ANY))
                {
                    if (baseReg.match(ESP))
                        throw TSCompileError("you cannot use esp as index register", operandPos);

                    std::swap(baseReg, indexReg);
                }
                else
                {
                    baseReg = indexReg;
                    indexReg.reset();
                    indexMult.reset();
                }
            }
        }
        
        if (opMask.match(MEM_16))
        {
            if (baseReg.match(BX))
            {
                if (indexReg.match(SI))
                    opMask |= MEM_16_BX_SI;
                else if (indexReg.match(DI))
                    opMask |= MEM_16_BX_DI;
                else
                    opMask |= MEM_16_BX;
            }
            else if (baseReg.match(BP))
            {
                if (indexReg.match(SI))
                    opMask |= MEM_16_BP_SI;
                else if (indexReg.match(DI))
                    opMask |= MEM_16_BP_DI;
                else
                    opMask |= MEM_16_BP;
            }
            else if (baseReg.match(SI))
                opMask |= MEM_16_SI;
            else if (baseReg.match(DI))
                opMask |= MEM_16_DI;
        }
        else
        {
            Mask baseRegPureData = baseReg & OPVAL_ANY;
            Mask indexRegPureData = indexReg & OPVAL_ANY;

            if (baseRegPureData.any())
                opMask |= MEM_32 | MEM_BASE | baseRegPureData;

            if (indexRegPureData.any())
                opMask |= indexMult | (indexRegPureData << 8);
        }

        operandContainerVector.push_back(OperandContainer({opMask, disp}, operandPos));
    }
}

tuple<string, vector<string>> TSRawInstructionSentence::present() const
{
    string instructionStr = findByValue(TSInstruction::instructionMap, instruction)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
        operandStrVector.push_back(get<0>(*it).present());

    return make_tuple(instructionStr, operandStrVector);
}

string TSRawInstructionSentence::Operand::present() const
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
        {
            if (rawNum.num == 0)
                if (rawNum.labelStr)
                    innerMemStr += *rawNum.labelStr;
                else
                    innerMemStr += "0";
            else
            {
                innerMemStr += rawNum.num.str();

                if (rawNum.labelStr)
                    innerMemStr += "+" + *rawNum.labelStr;
            }
        }
        else
        {
            if (rawNum.num == 0)
            {
                if (rawNum.labelStr)
                    innerMemStr += "+" + *rawNum.labelStr;
            }
            else
            {
                innerMemStr += rawNum.num.str(true);

                if (rawNum.labelStr)
                    innerMemStr += "+" + *rawNum.labelStr;
            }
        }

        memStr += innerMemStr + "]";

        return memStr;
    }
    else
    {
        string immStr;
        
        if (mask.match(S8))
            immStr += "BYTE ";
        else if (mask.match(S16))
            immStr += "WORD ";
        else if (mask.match(S32))
            immStr += "DWORD ";

        immStr += rawNum.num.str();

        return immStr;
    }
}

TSRawDataSentence::TSRawDataSentence(const TSPseudoSentence &pseudoSentence) :
    TSRawSentence(pseudoSentence.baseTokenContainer.pos),
    dataIdentifier(pseudoSentence.baseTokenContainer.token.value<TSToken::DataIdentifier>())
{
    const vector<vector<TSTokenContainer>> &operandsTokenContainerVector = pseudoSentence.operandsTokenContainerVector;

    for (auto it = operandsTokenContainerVector.begin(); it != operandsTokenContainerVector.end(); ++it)
    {
        TSCodePosition operandPos = calculatePos(it->begin(), it->end());

        if ((it->size() == 1) &&
            ((*it)[0].token.type() == TSToken::Type::CONSTANT_STRING))
        {
            if (dataIdentifier != DataIdentifier::DB)
                throw TSCompileError("you can use string constant only with DB", operandPos);

            string str = (*it)[0].token.value<string>();

            for (size_t i = 0; i < str.size(); ++i)
            {
                TSCodePosition charPos = {operandPos.row, operandPos.column + i + 1, 1};
                operandContainerVector.push_back(OperandContainer({(TSUInt)str[i], nullopt}, charPos));
            }

            continue;
        }

        vector<vector<TSTokenContainer>> operandTokenContainerVectors;

        auto firstIt = it->begin();
        for (auto jt = it->begin(); jt != it->end(); ++jt)
        {
            if ((jt->token.type() == TSToken::Type::MATH_SYMBOL) &&
                (jt->token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::PLUS))
            {
                operandTokenContainerVectors.push_back({firstIt, jt});
                firstIt = jt + 1;

                if (firstIt == it->end())
                    throw TSCompileError("'+' must have right value", jt->pos);
            }
        }
        operandTokenContainerVectors.push_back({firstIt, it->end()});

        TSRawNumber rawNum;

        for (auto jt = operandTokenContainerVectors.begin(); jt != operandTokenContainerVectors.end(); ++jt)
        {
            if (!jt->empty())
            {
                auto kt = jt->begin();
                
                if (kt->token.type() == TSToken::Type::USER_IDENTIFIER)
                {
                    if (rawNum.labelStr)
                        throw TSCompileError("you can use only one pointer in data", kt->pos);

                    rawNum.labelStr = kt->token.value<string>();

                    ++kt;
                }

                if (kt != jt->end())
                {
                    pureMathThrowableCheck(kt, jt->end());

                    rawNum.num += mathExpressionComputer(convertToMathOperationVector({kt, jt->end()}));
                }
            }
        }

        operandContainerVector.push_back(OperandContainer(rawNum, operandPos));
    }
}

tuple<string, vector<string>> TSRawDataSentence::present() const
{
    string instructionStr = findByValue(TSInstruction::dataIdentifierMap, dataIdentifier)->first;

    vector<string> operandStrVector;
    for (auto it = operandContainerVector.begin(); it != operandContainerVector.end(); ++it)
    {
        const TSRawNumber &rawNum = get<0>(*it);
        
        string str;
        if (rawNum.num == 0)
        {
            if (rawNum.labelStr)
                str += *rawNum.labelStr;
            else
                str += "0";
        }
        else
        {
            str += rawNum.num.str();

            if (rawNum.labelStr)
                str += "+" + *rawNum.labelStr;
        }

        operandStrVector.push_back(str);
    }

    return make_tuple(instructionStr, operandStrVector);
}

vector<TSRawSentencesSegmentContainer> constructRawSentences(const vector<TSPseudoSentencesSegmentContainer> &pseudoSentencesSegmentContainerVector)
{
    vector<TSRawSentencesSegmentContainer> rawSentencesSegmentContainerVector;

    for (auto it = pseudoSentencesSegmentContainerVector.begin(); it != pseudoSentencesSegmentContainerVector.end(); ++it)
    {
        vector<shared_ptr<TSRawSentence>> rawSentenceVector;

        for (auto jt = get<1>(*it).begin(); jt != get<1>(*it).end(); ++jt)
        {
            if (jt->baseTokenContainer.token.type() == TSToken::Type::INSTRUCTION)
                rawSentenceVector.push_back(shared_ptr<TSRawSentence>(new TSRawInstructionSentence(*jt)));
            else
                rawSentenceVector.push_back(shared_ptr<TSRawSentence>(new TSRawDataSentence(*jt)));
        }

        rawSentencesSegmentContainerVector.push_back(make_tuple(get<0>(*it), rawSentenceVector));
    }

    return rawSentencesSegmentContainerVector;
}
