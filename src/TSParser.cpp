#include "TSParser.h"

#include "TSException.h"
#include "TSMath.h"
#include "TSDiagnostics.h"
#include <functional>
#include <algorithm>

vector<TSMathOperation> convertToMathOperationVector(const vector<TSTokenContainer> &tokenContainerVector, const map<string, longlong> &equMap)
{
    vector<TSMathOperation> mathOperationVector;

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        const TSTokenContainer &tokenContainer = *it;
        const TSToken &token = tokenContainer.token;

        TSMathOperation currentOperation;

        if (token.type() == TSToken::Type::mathSymbol)
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
                                tokenContainer.row,
                                tokenContainer.column,
                                tokenContainer.length};
        }
        else if (token.type() == TSToken::Type::userIdentifier)
        {
            currentOperation = {TSMathOperationKind::CONSTANT,
                                equMap.find(token.value<string>())->second,
                                tokenContainer.row,
                                tokenContainer.column,
                                tokenContainer.length};
        }
        else if (token.type() == TSToken::Type::constantNumber)
        {
            currentOperation = {TSMathOperationKind::CONSTANT,
                                token.value<longlong>(),
                                tokenContainer.row,
                                tokenContainer.column,
                                tokenContainer.length};
        }

        mathOperationVector.push_back(currentOperation);
    }

    return mathOperationVector;
}

inline longlong computeMath(const vector<TSTokenContainer> &tokenContainerVector, const map<string, longlong> &equMap)
{
    return mathExpressionComputer(convertToMathOperationVector(tokenContainerVector, equMap));
}

vector<TSTokenContainer> excludeUsedTokens(const vector<TSTokenContainer> &base, const vector<TSTokenContainer> &excludes)
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
        return (token.type() == TSToken::Type::mathSymbol) &&
               ((token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::PLUS) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::MINUS) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::MULTIPLY) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::DIVIDE) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::BRACKET_OPEN));
    };
    
    auto it = begin;
    while ((it != end) &&
           ((it->token.type() == TSToken::Type::mathSymbol) ||
            ((it->token.type() == TSToken::Type::userIdentifier) &&
             ((requireRightParam((it - 1)->token)) ||
              (it == begin))) ||
            ((it->token.type() == TSToken::Type::constantNumber) &&
             ((requireRightParam((it - 1)->token)) ||
              (it == begin)))))
    {
        ++it;
    }

    return it;
}

tuple<map<string, longlong>, vector<TSTokenContainer>> processEQUs(const vector<TSTokenContainer> &tokenContainerVector)
{
    vector<TSTokenContainer> excludes;

    map<string, vector<TSTokenContainer>> equMapUnprocessed;

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it)
    {
        const TSTokenContainer &tokenContainer = *it;
        const TSToken &token = tokenContainer.token;

        if ((token.type() == TSToken::Type::directive) && (token.value<TSToken::Directive>() == TSToken::Directive::EQU))
        {
            excludes.push_back(tokenContainer);

            auto equTokenEnd = getMathTokenSequence(it + 1, tokenContainerVector.end());
            vector<TSTokenContainer> currentEquTokenContainer(it + 1, equTokenEnd);
            excludes.insert(excludes.end(), it + 1, equTokenEnd);

            if (currentEquTokenContainer.empty())
                throw TSCompileError("EQU must have an integer value", tokenContainer);

            if ((it == tokenContainerVector.begin()) ||
                ((it - 1)->token.type() != TSToken::Type::userIdentifier))
                throw TSCompileError("EQU must define an user identifier", tokenContainer);

            excludes.push_back(*(it - 1));

            if (equMapUnprocessed.count((it - 1)->token.value<string>()))
                throw TSCompileError("redefinition of EQU constant is illegal", tokenContainer);

            equMapUnprocessed[(it - 1)->token.value<string>()] = currentEquTokenContainer;
        }
    }

    map<string, longlong> equMap;

    std::function<void(string, vector<string> &)> recursiveEquComputer = [&](string equName, vector<string> &recProtect) {
        if (!equMap.count(equName))
        {
            vector<TSTokenContainer> currentEquTokenContainer(equMapUnprocessed[equName]);

            for (auto it = currentEquTokenContainer.begin(); it != currentEquTokenContainer.end(); ++it)
            {
                const TSTokenContainer &tokenContainer = *it;
                const TSToken &token = tokenContainer.token;
                
                if (token.type() == TSToken::Type::userIdentifier)
                {
                    if (equMapUnprocessed.count(token.value<string>()))
                    {
                        if (!equMap.count(token.value<string>()))
                        {
                            if (std::find(recProtect.begin(), recProtect.end(), token.value<string>()) != recProtect.end())
                                throw TSCompileError("recursive EQUs are impossible", tokenContainer);
                            
                            recProtect.push_back(token.value<string>());
                            recursiveEquComputer(token.value<string>(), recProtect);
                        }
                    }
                    else
                        throw TSCompileError("identifier is not constant", tokenContainer);
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

    return tuple<map<string, longlong>, vector<TSTokenContainer>>(equMap, excludeUsedTokens(tokenContainerVector, excludes));
}

vector<TSTokenContainer> processIFs(const vector<TSTokenContainer> &tokenContainerVector, const map<string, longlong> &equMap)
{
    vector<TSTokenContainer> excludes;

    typedef vector<TSTokenContainer>::const_iterator ItType;

    std::function<void(ItType, ItType, bool)> ifContentAnalyzer;

    std::function<void(ItType, ItType)> ifAnalyzer = [&](ItType begin, ItType end) {
        ItType it = begin;
        size_t ifCount = 0;
        ItType ifIt = end;
        while (it != end)
        {
            if (it->token.type() == TSToken::Type::conditionDirective)
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
                        throw TSCompileError("must only be after IF", *it);
                }
                else
                {
                    if (ifCount == 0)
                        throw TSCompileError("must only be after IF", *it);
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
                            throw TSCompileError("IF must compare int value", *(jt - 1));

                        longlong leftNumber = computeMath(vector<TSTokenContainer>(jt, leftExprEnd), equMap);
                        excludes.insert(excludes.end(), jt, leftExprEnd);
                        jt = leftExprEnd;

                        if (jt->token.type() != TSToken::Type::condition)
                            throw TSCompileError("there must be condition after expression", *(jt - 1));

                        TSToken::Condition condition = jt->token.value<TSToken::Condition>();
                        excludes.push_back(*jt);
                        ++jt;

                        ItType rightExprEnd = getMathTokenSequence(jt, it);
                        if (rightExprEnd == jt)
                            throw TSCompileError("IF must compare to int value", *(jt - 1));

                        longlong rightNumber = computeMath(vector<TSTokenContainer>(jt, rightExprEnd), equMap);
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
            else if (it->token.type() == TSToken::Type::condition)
            {
                if (ifCount == 0)
                    throw TSCompileError("conditions without IF is illegal", *it);
            }

            ++it;
        }

        if (ifCount != 0)
            throw TSCompileError("unclosed IF", *ifIt);
    };

    ifContentAnalyzer = [&](ItType begin, ItType end, bool isConditionTrue) {
        ItType it = begin;
        size_t ifCount = 0;
        ItType elseIt = end;
        while (it != end)
        {
            
            if (it->token.type() == TSToken::Type::conditionDirective)
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

void parse(const vector<TSTokenContainer> &tokenContainerVector)
{
    auto firstPhaseResult = processEQUs(tokenContainerVector);
    auto secondPhaseResult = processIFs(std::get<1>(firstPhaseResult), std::get<0>(firstPhaseResult));
    printTokenTable(secondPhaseResult);
    //printEquTable(std::get<0>(firstPhaseResult));
}
