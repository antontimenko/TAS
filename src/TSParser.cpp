#include "TSParser.h"

#include "TSException.h"
#include "TSMath.h"
#include "TSDiagnostics.h"
#include <functional>

map<string, vector<TSTokenContainer>> constructNativeEquMap(const vector<TSTokenContainer> &tokenContainerVector)
{
    map<string, vector<TSTokenContainer>> nativeEquMap;

    for (size_t i = 0; i < tokenContainerVector.size(); ++i)
    {
        const TSTokenContainer &tokenContainer = tokenContainerVector[i];
        const TSToken &token = tokenContainer.token;

        if ((token.type() == TSToken::Type::directive) && (token.value<TSToken::Directive>() == TSToken::Directive::EQU))
        {
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

            if (nativeEquMap.count(tokenContainerVector[i - 1].token.value<string>()))
                throw TSCompileError("redefinition of EQU constant",
                                     tokenContainer.row,
                                     tokenContainer.column,
                                     tokenContainer.length);

            nativeEquMap[tokenContainerVector[i - 1].token.value<string>()] = currentEquTokenContainer;
        }
    }

    return nativeEquMap;
}

map<string, longlong> constructEquMap(map<string, vector<TSTokenContainer>> nativeEquMap)
{
    map<string, longlong> equMap;

    std::function<void(string, vector<string> &)> recursiveEquComputer = [&](string equName, vector<string> &recProtect) {
        if (!equMap.count(equName))
        {
            vector<TSTokenContainer> currentEquTokenContainer(nativeEquMap[equName]);

            for (size_t i = 0; i < currentEquTokenContainer.size(); ++i)
            {
                const TSTokenContainer &tokenContainer = currentEquTokenContainer[i];
                const TSToken &token = tokenContainer.token;
                
                if (token.type() == TSToken::Type::userIdentifier)
                {
                    if (nativeEquMap.count(token.value<string>()))
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

            equMap[equName] = mathExpressionComputer(convertToMathOperationVector(nativeEquMap[equName], equMap));
        }
    };

    for (auto it = nativeEquMap.begin(); it != nativeEquMap.end(); ++it)
    {
        vector<string> recProtect;
        recProtect.push_back(it->first);
        recursiveEquComputer(it->first, recProtect);
    }

    return equMap;
}

void parse(const vector<TSTokenContainer> &tokenContainerVector)
{
    map<string, longlong> equMap = constructEquMap(constructNativeEquMap(tokenContainerVector));

    printEquTable(equMap);
}
