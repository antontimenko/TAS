#include "TSPreprocessor.h"

#include "TSException.h"
#include "TSInteger.h"
#include "TSMath.h"
#include "TSDiagnostics.h"
#include <functional>
#include <algorithm>

vector<TSMathOperation> convertToMathOperationVector(const vector<TSTokenContainer> &tokenContainerVector, const map<string, TSInteger> &equMap)
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
                                0_I,
                                tokenContainer.pos};
        }
        else if (token.type() == TSToken::Type::USER_IDENTIFIER)
        {
            currentOperation = {TSMathOperationKind::CONSTANT,
                                equMap.find(token.value<string>())->second,
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
}

TSInteger computeMath(const vector<TSTokenContainer> &tokenContainerVector, const map<string, TSInteger> equMap)
{
    return mathExpressionComputer(convertToMathOperationVector(tokenContainerVector, equMap));
}

auto excludeUsedTokens(const vector<TSTokenContainer> &base, const vector<TSTokenContainer> &excludes)
{
    vector<TSTokenContainer> newBase = base;

    for (auto it = excludes.begin(); it != excludes.end(); ++it)
    {
        auto elem = std::find(newBase.begin(), newBase.end(), *it);
        if (elem != newBase.end())
            newBase.erase(elem);
    }

    return newBase;
}

vector<TSTokenContainer>::const_iterator getMathTokenSequence(vector<TSTokenContainer>::const_iterator begin, vector<TSTokenContainer>::const_iterator end)
{
    auto requireRightParam = [](const TSToken &token) -> bool {
        return (token.type() == TSToken::Type::MATH_SYMBOL) &&
               (token.value<TSToken::MathSymbol>() != TSToken::MathSymbol::BRACKET_CLOSE);
    };
    
    auto it = begin;
    while ((it != end) &&
           ((it->token.type() == TSToken::Type::MATH_SYMBOL) ||
            ((it->token.type() == TSToken::Type::USER_IDENTIFIER) &&
             ((it == begin) ||
              (requireRightParam((it - 1)->token)))) ||
            ((it->token.type() == TSToken::Type::CONSTANT_NUMBER) &&
             ((it == begin) ||
              (requireRightParam((it - 1)->token))))))
    {
        ++it;
    }

    return it;
}

auto processEQUs(const vector<TSTokenContainer> &tokenContainerVector)
{
    vector<TSTokenContainer> excludes;

    map<string, vector<TSTokenContainer>> equMapUnprocessed;

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        const TSTokenContainer &tokenContainer = *it;
        const TSToken &token = tokenContainer.token;

        if (token.type() == TSToken::Type::EQU_DIRECTIVE)
        {
            excludes.push_back(tokenContainer);

            auto equTokenEnd = getMathTokenSequence(it + 1, tokenContainerVector.end());
            vector<TSTokenContainer> currentEquTokenContainer(it + 1, equTokenEnd);
            excludes.insert(excludes.end(), it + 1, equTokenEnd);

            if (currentEquTokenContainer.empty())
                throw TSCompileError("EQU must have an integer value", tokenContainer.pos);

            if ((it == tokenContainerVector.begin()) ||
                ((it - 1)->token.type() != TSToken::Type::USER_IDENTIFIER))
                throw TSCompileError("EQU must define an user identifier", tokenContainer.pos);

            excludes.push_back(*(it - 1));

            if (equMapUnprocessed.count((it - 1)->token.value<string>()))
                throw TSCompileError("redefinition of EQU constant is illegal", tokenContainer.pos);

            equMapUnprocessed[(it - 1)->token.value<string>()] = currentEquTokenContainer;
        }
    }

    map<string, TSInteger> equMap;

    function<void(string, vector<string> &)> recursiveEquComputer = [&](string equName, vector<string> &recProtect) {
        if (!equMap.count(equName))
        {
            vector<TSTokenContainer> currentEquTokenContainer(equMapUnprocessed[equName]);

            for (auto it = currentEquTokenContainer.begin(); it != currentEquTokenContainer.end(); ++it)
            {
                const TSTokenContainer &tokenContainer = *it;
                const TSToken &token = tokenContainer.token;
                
                if (token.type() == TSToken::Type::USER_IDENTIFIER)
                {
                    if (equMapUnprocessed.count(token.value<string>()))
                    {
                        if (!equMap.count(token.value<string>()))
                        {
                            if (std::find(recProtect.begin(), recProtect.end(), token.value<string>()) != recProtect.end())
                                throw TSCompileError("recursive EQUs are impossible", tokenContainer.pos);
                            
                            recProtect.push_back(token.value<string>());
                            recursiveEquComputer(token.value<string>(), recProtect);
                        }
                    }
                    else
                        throw TSCompileError("identifier is not constant", tokenContainer.pos);
                }
            }

            equMap[equName] = computeMath(equMapUnprocessed[equName], equMap);
        }
    };

    for (auto it = equMapUnprocessed.begin(); it != equMapUnprocessed.end(); ++it)
    {
        vector<string> recProtect;
        recProtect.push_back(it->first);
        recursiveEquComputer(it->first, recProtect);
    }

    return make_tuple(equMap, excludeUsedTokens(tokenContainerVector, excludes));
}

auto processIFs(const vector<TSTokenContainer> &tokenContainerVector, const map<string, TSInteger> &equMap)
{
    vector<TSTokenContainer> excludes;

    typedef vector<TSTokenContainer>::const_iterator ItType;

    function<void(ItType, ItType, bool)> ifContentAnalyzer;

    function<void(ItType, ItType)> ifAnalyzer = [&](ItType begin, ItType end) {
        ItType it = begin;
        size_t ifCount = 0;
        ItType ifIt = end;
        while (it != end)
        {
            if (it->token.type() == TSToken::Type::CONDITION_DIRECTIVE)
            {
                if (it->token.value<TSToken::ConditionDirective>() == TSToken::ConditionDirective::IF)
                {
                    if (ifCount == 0)
                        ifIt = it;
                    ++ifCount;
                }
                else if (it->token.value<TSToken::ConditionDirective>() == TSToken::ConditionDirective::ELSE)
                {
                    if (ifCount == 0)
                        throw TSCompileError("must only be after IF", it->pos);
                }
                else
                {
                    if (ifCount == 0)
                        throw TSCompileError("must only be after IF", it->pos);
                    else if (ifCount > 1)
                        --ifCount;
                    else
                    {
                        excludes.push_back(*it);

                        ItType jt = ifIt;

                        excludes.push_back(*jt);
                        ++jt;

                        ItType leftExprEnd = getMathTokenSequence(jt, it);
                        if (leftExprEnd == jt)
                            throw TSCompileError("IF must compare int value", (jt - 1)->pos);

                        TSInteger leftNumber = computeMath(vector<TSTokenContainer>(jt, leftExprEnd), equMap);
                        excludes.insert(excludes.end(), jt, leftExprEnd);
                        jt = leftExprEnd;

                        if (jt->token.type() != TSToken::Type::CONDITION)
                            throw TSCompileError("there must be condition after expression", (jt - 1)->pos);

                        TSToken::Condition condition = jt->token.value<TSToken::Condition>();
                        excludes.push_back(*jt);
                        ++jt;

                        ItType rightExprEnd = getMathTokenSequence(jt, it);
                        if (rightExprEnd == jt)
                            throw TSCompileError("IF must compare to int value", (jt - 1)->pos);

                        TSInteger rightNumber = computeMath(vector<TSTokenContainer>(jt, rightExprEnd), equMap);
                        excludes.insert(excludes.end(), jt, rightExprEnd);
                        jt = rightExprEnd;

                        bool conditionIsTrue;
                        switch (condition)
                        {
                        case TSToken::Condition::EQ:
                            conditionIsTrue = (leftNumber == rightNumber);
                            break;
                        case TSToken::Condition::NE:
                            conditionIsTrue = (leftNumber != rightNumber);
                            break;
                        case TSToken::Condition::LT:
                            conditionIsTrue = (leftNumber < rightNumber);
                            break;
                        case TSToken::Condition::LE:
                            conditionIsTrue = (leftNumber <= rightNumber);
                            break;
                        case TSToken::Condition::GT:
                            conditionIsTrue = (leftNumber > rightNumber);
                            break;
                        case TSToken::Condition::GE:
                            conditionIsTrue = (leftNumber >= rightNumber);
                            break;
                        }

                        ifContentAnalyzer(jt, it, conditionIsTrue);

                        --ifCount;
                        ifIt = end;
                    }
                }
            }
            else if (it->token.type() == TSToken::Type::CONDITION)
            {
                if (ifCount == 0)
                    throw TSCompileError("conditions without IF is illegal", it->pos);
            }

            ++it;
        }

        if (ifCount != 0)
            throw TSCompileError("unclosed IF", ifIt->pos);
    };

    ifContentAnalyzer = [&](ItType begin, ItType end, bool isConditionTrue) {
        ItType it = begin;
        size_t ifCount = 0;
        ItType elseIt = end;
        while (it != end)
        {
            
            if (it->token.type() == TSToken::Type::CONDITION_DIRECTIVE)
            {
                if (it->token.value<TSToken::ConditionDirective>() == TSToken::ConditionDirective::IF)
                    ++ifCount;
                else if (it->token.value<TSToken::ConditionDirective>() == TSToken::ConditionDirective::ELSE)
                {
                    if (ifCount == 0)
                    {
                        elseIt = it;
                        break;
                    }
                }
                else
                {
                    if (ifCount > 0)
                        --ifCount;
                }
            }

            ++it;
        }

        if (elseIt == end)
        {
            if (isConditionTrue)
                ifAnalyzer(begin, end);
            else
                excludes.insert(excludes.end(), begin, end);
        }
        else
        {
            if (isConditionTrue)
            {
                excludes.insert(excludes.end(), elseIt, end);
                ifAnalyzer(begin, elseIt);
            }
            else
            {
                excludes.insert(excludes.end(), begin, elseIt + 1);
                ifAnalyzer(elseIt + 1, end);
            }
        }
    };

    ifAnalyzer(tokenContainerVector.begin(), tokenContainerVector.end());

    return excludeUsedTokens(tokenContainerVector, excludes);
}

auto processSymbolicConstantReplace(vector<TSTokenContainer> tokenContainerVector, const map<string, TSInteger> &equMap)
{
    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        if (it->token.type() == TSToken::Type::USER_IDENTIFIER)
        {
            string identifier = it->token.value<string>();
            if (equMap.count(identifier))
                *it = {it->pos, TSToken(TSToken::Type::CONSTANT_NUMBER, equMap.find(identifier)->second)};
        }
    }

    return tokenContainerVector;
}

auto processSegmentsParting(const vector<TSTokenContainer> &tokenContainerVector)
{
    vector<TSTokenContainersSegmentContainer> segmentTokenContainerVector;

    vector<TSTokenContainer> excludes;

    auto segmentStartIt = tokenContainerVector.end();
    for (auto it = tokenContainerVector.begin(); it < tokenContainerVector.end(); ++it)
    {
        if (it->token.type() == TSToken::Type::SEGMENT_DIRECTIVE)
        {
            if (it->token.value<TSToken::SegmentDirective>() == TSToken::SegmentDirective::SEGMENT)
            {
                if (segmentStartIt == tokenContainerVector.end())
                {
                    if ((it != tokenContainerVector.begin()) &&
                        ((it - 1)->token.type() == TSToken::Type::USER_IDENTIFIER))
                        segmentStartIt = it;
                    else
                        throw TSCompileError("SEGMENT must have a name", it->pos);
                }
                else
                    throw TSCompileError("you cannot declare a segment inside another one", it->pos);
            }
            else
            {
                if (segmentStartIt != tokenContainerVector.end())
                {
                    if ((it != tokenContainerVector.begin()) &&
                        ((it - 1)->token.type() == TSToken::Type::USER_IDENTIFIER) &&
                        ((it - 1)->token.value<string>() == (segmentStartIt - 1)->token.value<string>()))
                    {
                        segmentTokenContainerVector.push_back(make_tuple((it - 1)->token.value<string>(),
                                                                         vector<TSTokenContainer>(segmentStartIt + 1, it - 1)));

                        excludes.insert(excludes.end(), segmentStartIt - 1, it + 1);

                        segmentStartIt = tokenContainerVector.end();
                    }
                    else
                        throw TSCompileError("ENDS require a name", it->pos);
                }
                else
                    throw TSCompileError("ENDS must end a SEGMENT", it->pos);
            }
        }
    }

    vector<TSTokenContainer> remains = excludeUsedTokens(tokenContainerVector, excludes);

    if ((remains.end() - 1)->token.type() == TSToken::Type::END_DIRECTIVE)
        remains.erase(remains.end() - 1);
    else if (((remains.end() - 1)->token.type() == TSToken::Type::USER_IDENTIFIER) &&
             ((remains.end() - 2)->token.type() == TSToken::Type::END_DIRECTIVE))
        remains.erase(remains.end() - 2, remains.end());

    if (!remains.empty())
        throw TSCompileError("undefined expression outside segment", remains.begin()->pos);

    return segmentTokenContainerVector;
}

void checkSegmentNameExistance(const vector<TSTokenContainersSegmentContainer> &segmentTokenContainerVector)
{
    vector<string> segmentNameVector;
    for (auto it = segmentTokenContainerVector.begin(); it != segmentTokenContainerVector.end(); ++it)
        segmentNameVector.push_back(get<0>(*it));

    for (auto it = segmentTokenContainerVector.begin(); it != segmentTokenContainerVector.end(); ++it)
    {
        for (auto jt = get<1>(*it).begin(); jt != get<1>(*it).end(); ++jt)
        {
            if ((jt->token.type() == TSToken::Type::USER_IDENTIFIER) &&
                (std::count(segmentNameVector.begin(), segmentNameVector.end(), jt->token.value<string>())))
            {
                throw TSCompileError("segment name usage is not implemented yet", jt->pos);
            }
        }
    }
}

vector<TSTokenContainersSegmentContainer> preprocess(const vector<TSTokenContainer> &tokenContainerVector)
{
    auto equPhaseResult = processEQUs(tokenContainerVector);
    auto ifPhaseResult = processIFs(get<1>(equPhaseResult), get<0>(equPhaseResult));
    auto constantReplaceResult = processSymbolicConstantReplace(ifPhaseResult, get<0>(equPhaseResult));
    auto segmentsPartingResult = processSegmentsParting(constantReplaceResult);
    checkSegmentNameExistance(segmentsPartingResult);
    return segmentsPartingResult;
}
