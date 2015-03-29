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
        {
            newBase.erase(elem);
        }
    }

    return newBase;
}

size_t getMathTokenSequence(const vector<TSTokenContainer> &tokenContainerVector, vector<TSTokenContainer>::const_iterator it)
{
    auto requireRightParam = [](const TSToken &token) -> bool {
        return (token.type() == TSToken::Type::mathSymbol) &&
               ((token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::PLUS) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::MINUS) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::MULTIPLY) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::DIVIDE) ||
                (token.value<TSToken::MathSymbol>() == TSToken::MathSymbol::BRACKET_OPEN));
    };

    size_t i = 0;
    
    while ((it != tokenContainerVector.end()) &&
           ((it->token.type() == TSToken::Type::mathSymbol) ||
            ((it->token.type() == TSToken::Type::userIdentifier) &&
             ((requireRightParam((it - 1)->token)) ||
              (i == 0))) ||
            ((it->token.type() == TSToken::Type::constantNumber) &&
             ((requireRightParam((it - 1)->token)) ||
              (i == 0)))))
    {
        ++it;
        ++i;
    }

    return i;
}

tuple<map<string, longlong>, vector<TSTokenContainer>> constructEquMap(const vector<TSTokenContainer> &tokenContainerVector)
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

            size_t equTokenNum = getMathTokenSequence(tokenContainerVector, it + 1);
            vector<TSTokenContainer> currentEquTokenContainer(it + 1, it + equTokenNum + 1);

            if (currentEquTokenContainer.empty())
                throw TSCompileError("EQU must have an integer value",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

            if ((it == tokenContainerVector.begin()) ||
                ((it - 1)->token.type() != TSToken::Type::userIdentifier))
                throw TSCompileError("EQU must define an user identifier",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

            excludes.push_back(*(it - 1));

            if (equMapUnprocessed.count((it - 1)->token.value<string>()))
                throw TSCompileError("redefinition of EQU constant",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

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
                                throw TSCompileError("recursive EQUs are impossible",
                                                      tokenContainer.row,
                                                      tokenContainer.column,
                                                      tokenContainer.length);
                            
                            recProtect.push_back(token.value<string>());
                            recursiveEquComputer(token.value<string>(), recProtect);
                        }
                    }
                    else
                        throw TSCompileError("identifier not constant",
                                             tokenContainer.row,
                                             tokenContainer.column,
                                             tokenContainer.length);
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

void parse(const vector<TSTokenContainer> &tokenContainerVector)
{
    auto firstPhaseResult = constructEquMap(tokenContainerVector);

    printEquTable(std::get<0>(firstPhaseResult));
}
