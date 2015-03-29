#include "TSParser.h"

#include "TSException.h"
#include "TSMath.h"
#include "TSDiagnostics.h"
#include <functional>
#include <algorithm>

vector<TSMathOperation> convertToMathOperationVector(const vector<TSTokenContainer> &tokenContainerVector, const map<string, longlong> &equMap)
{
    vector<TSMathOperation> mathOperationVector;

    for (size_t i = 0; i < tokenContainerVector.size(); ++i)
    {
        const TSTokenContainer &tokenContainer = tokenContainerVector[i];
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

vector<TSTokenContainer> excludeUsedTokens(const vector<TSTokenContainer> &base, const vector<TSTokenContainer> &excludes)
{
    vector<TSTokenContainer> newBase = base;

    for (size_t i = 0; i < excludes.size(); ++i)
    {
        auto elem = std::find(newBase.begin(), newBase.end(), excludes[i]);
        if (elem != newBase.end())
        {
            newBase.erase(elem);
        }
    }

    return newBase;
}

tuple<map<string, longlong>, vector<TSTokenContainer>> constructEquMap(const vector<TSTokenContainer> &tokenContainerVector)
{
    vector<TSTokenContainer> excludes;

    map<string, vector<TSTokenContainer>> equMapUnprocessed;

    for (size_t i = 0; i < tokenContainerVector.size(); ++i)
    {
        const TSTokenContainer &tokenContainer = tokenContainerVector[i];
        const TSToken &token = tokenContainer.token;

        if ((token.type() == TSToken::Type::directive) && (token.value<TSToken::Directive>() == TSToken::Directive::EQU))
        {
            excludes.push_back(tokenContainer);

            vector<TSTokenContainer> currentEquTokenContainer;

            size_t j = i + 1;
            while ((j < tokenContainerVector.size()) && 
                   ((tokenContainerVector[j].token.type() == TSToken::Type::mathSymbol) || 
                    ((tokenContainerVector[j].token.type() == TSToken::Type::userIdentifier) &&
                     ((TSToken::isMathSymbolRightCompatible(tokenContainerVector[j - 1].token)) || 
                      (currentEquTokenContainer.empty()))) ||
                    ((tokenContainerVector[j].token.type() == TSToken::Type::constantNumber) &&
                     ((TSToken::isMathSymbolRightCompatible(tokenContainerVector[j - 1].token)) || 
                      (currentEquTokenContainer.empty())))))
            {
                excludes.push_back(tokenContainerVector[j]);

                currentEquTokenContainer.push_back(tokenContainerVector[j]);

                ++j;
            }

            if (currentEquTokenContainer.empty())
                throw TSCompileError("EQU must have an integer value",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

            if ((i == 0) || (tokenContainerVector[i - 1].token.type() != TSToken::Type::userIdentifier))
                throw TSCompileError("EQU must define an user identifier",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

            excludes.push_back(tokenContainerVector[i - 1]);

            if (equMapUnprocessed.count(tokenContainerVector[i - 1].token.value<string>()))
                throw TSCompileError("redefinition of EQU constant",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

            equMapUnprocessed[tokenContainerVector[i - 1].token.value<string>()] = currentEquTokenContainer;
        }
    }

    map<string, longlong> equMap;

    std::function<void(string, vector<string> &)> recursiveEquComputer = [&](string equName, vector<string> &recProtect) {
        if (!equMap.count(equName))
        {
            vector<TSTokenContainer> currentEquTokenContainer(equMapUnprocessed[equName]);

            for (size_t i = 0; i < currentEquTokenContainer.size(); ++i)
            {
                const TSTokenContainer &tokenContainer = currentEquTokenContainer[i];
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

            equMap[equName] = mathExpressionComputer(convertToMathOperationVector(equMapUnprocessed[equName], equMap));
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
