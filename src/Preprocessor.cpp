#include "Preprocessor.h"

#include "Exception.h"
#include "Integer.h"
#include "Math.h"
#include <algorithm>

vector<MathOperation> convertToMathOperationVector(const vector<TokenContainer> &tokenContainerVector, const map<string, Integer> &equMap) {
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
        } else if (token.type() == Token::Type::USER_IDENTIFIER) {
            currentOperation = {MathOperationKind::CONSTANT,
                                equMap.find(token.value<string>())->second,
                                tokenContainer.pos};
        } else if (token.type() == Token::Type::CONSTANT_NUMBER) {
            currentOperation = {MathOperationKind::CONSTANT,
                                token.value<Integer>(),
                                tokenContainer.pos};
        }

        mathOperationVector.push_back(currentOperation);
    }

    return mathOperationVector;
}

Integer computeMath(const vector<TokenContainer> &tokenContainerVector, const map<string, Integer> equMap) {
    return mathExpressionComputer(convertToMathOperationVector(tokenContainerVector, equMap));
}

auto excludeUsedTokens(const vector<TokenContainer> &base, const vector<TokenContainer> &excludes) {
    vector<TokenContainer> newBase = base;

    for (auto it = excludes.begin(); it != excludes.end(); ++it) {
        auto elem = std::find(newBase.begin(), newBase.end(), *it);
        if (elem != newBase.end())
            newBase.erase(elem);
    }

    return newBase;
}

vector<TokenContainer>::const_iterator getMathTokenSequence(vector<TokenContainer>::const_iterator begin, vector<TokenContainer>::const_iterator end) {
    auto requireRightParam = [](const Token &token) -> bool {
        return (token.type() == Token::Type::MATH_SYMBOL) &&
               (token.value<Token::MathSymbol>() != Token::MathSymbol::BRACKET_CLOSE);
    };
    
    auto it = begin;
    while ((it != end) &&
           ((it->token.type() == Token::Type::MATH_SYMBOL) ||
            ((it->token.type() == Token::Type::USER_IDENTIFIER) &&
             ((it == begin) ||
              (requireRightParam((it - 1)->token)))) ||
            ((it->token.type() == Token::Type::CONSTANT_NUMBER) &&
             ((it == begin) ||
              (requireRightParam((it - 1)->token))))))
    {
        ++it;
    }

    return it;
}

auto processEQUs(const vector<TokenContainer> &tokenContainerVector) {
    vector<TokenContainer> excludes;

    map<string, vector<TokenContainer>> equMapUnprocessed;

    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it) {
        const TokenContainer &tokenContainer = *it;
        const Token &token = tokenContainer.token;

        if (token.type() == Token::Type::EQU_DIRECTIVE) {
            excludes.push_back(tokenContainer);

            auto equTokenEnd = getMathTokenSequence(it + 1, tokenContainerVector.end());
            vector<TokenContainer> currentEquTokenContainer(it + 1, equTokenEnd);
            excludes.insert(excludes.end(), it + 1, equTokenEnd);

            if (currentEquTokenContainer.empty())
                throw CompileError("EQU must have an integer value", tokenContainer.pos);

            if ((it == tokenContainerVector.begin()) ||
                ((it - 1)->token.type() != Token::Type::USER_IDENTIFIER))
                throw CompileError("EQU must define an user identifier", tokenContainer.pos);

            excludes.push_back(*(it - 1));

            if (equMapUnprocessed.count((it - 1)->token.value<string>()))
                throw CompileError("redefinition of EQU constant is illegal", tokenContainer.pos);

            equMapUnprocessed[(it - 1)->token.value<string>()] = currentEquTokenContainer;
        }
    }

    map<string, Integer> equMap;

    function<void(string, vector<string> &)> recursiveEquComputer = [&](string equName, vector<string> &recProtect) {
        if (!equMap.count(equName)) {
            vector<TokenContainer> currentEquTokenContainer(equMapUnprocessed[equName]);

            for (auto it = currentEquTokenContainer.begin(); it != currentEquTokenContainer.end(); ++it) {
                const TokenContainer &tokenContainer = *it;
                const Token &token = tokenContainer.token;
                
                if (token.type() == Token::Type::USER_IDENTIFIER) {
                    if (equMapUnprocessed.count(token.value<string>())) {
                        if (!equMap.count(token.value<string>())) {
                            if (std::find(recProtect.begin(), recProtect.end(), token.value<string>()) != recProtect.end())
                                throw CompileError("recursive EQUs are impossible", tokenContainer.pos);
                            
                            recProtect.push_back(token.value<string>());
                            recursiveEquComputer(token.value<string>(), recProtect);
                        }
                    } else
                        throw CompileError("identifier is not constant", tokenContainer.pos);
                }
            }

            equMap[equName] = computeMath(equMapUnprocessed[equName], equMap);
        }
    };

    for (auto it = equMapUnprocessed.begin(); it != equMapUnprocessed.end(); ++it) {
        vector<string> recProtect;
        recProtect.push_back(it->first);
        recursiveEquComputer(it->first, recProtect);
    }

    return make_tuple(equMap, excludeUsedTokens(tokenContainerVector, excludes));
}

auto processIFs(const vector<TokenContainer> &tokenContainerVector, const map<string, Integer> &equMap) {
    vector<TokenContainer> excludes;

    typedef vector<TokenContainer>::const_iterator ItType;

    function<void(ItType, ItType, bool)> ifContentAnalyzer;

    function<void(ItType, ItType)> ifAnalyzer = [&](ItType begin, ItType end) {
        ItType it = begin;
        size_t ifCount = 0;
        ItType ifIt = end;
        while (it != end) {
            if (it->token.type() == Token::Type::CONDITION_DIRECTIVE) {
                if (it->token.value<Token::ConditionDirective>() == Token::ConditionDirective::IF) {
                    if (ifCount == 0)
                        ifIt = it;
                    ++ifCount;
                } else if (it->token.value<Token::ConditionDirective>() == Token::ConditionDirective::ELSE) {
                    if (ifCount == 0)
                        throw CompileError("must only be after IF", it->pos);
                } else {
                    if (ifCount == 0)
                        throw CompileError("must only be after IF", it->pos);
                    else if (ifCount > 1)
                        --ifCount;
                    else {
                        excludes.push_back(*it);

                        ItType jt = ifIt;

                        excludes.push_back(*jt);
                        ++jt;

                        ItType leftExprEnd = getMathTokenSequence(jt, it);
                        if (leftExprEnd == jt)
                            throw CompileError("IF must compare int value", (jt - 1)->pos);

                        Integer leftNumber = computeMath(vector<TokenContainer>(jt, leftExprEnd), equMap);
                        excludes.insert(excludes.end(), jt, leftExprEnd);
                        jt = leftExprEnd;

                        if (jt->token.type() != Token::Type::CONDITION)
                            throw CompileError("there must be condition after expression", (jt - 1)->pos);

                        Token::Condition condition = jt->token.value<Token::Condition>();
                        excludes.push_back(*jt);
                        ++jt;

                        ItType rightExprEnd = getMathTokenSequence(jt, it);
                        if (rightExprEnd == jt)
                            throw CompileError("IF must compare to int value", (jt - 1)->pos);

                        Integer rightNumber = computeMath(vector<TokenContainer>(jt, rightExprEnd), equMap);
                        excludes.insert(excludes.end(), jt, rightExprEnd);
                        jt = rightExprEnd;

                        bool conditionIsTrue;
                        switch (condition) {
                        case Token::Condition::EQ:
                            conditionIsTrue = (leftNumber == rightNumber);
                            break;
                        case Token::Condition::NE:
                            conditionIsTrue = (leftNumber != rightNumber);
                            break;
                        case Token::Condition::LT:
                            conditionIsTrue = (leftNumber < rightNumber);
                            break;
                        case Token::Condition::LE:
                            conditionIsTrue = (leftNumber <= rightNumber);
                            break;
                        case Token::Condition::GT:
                            conditionIsTrue = (leftNumber > rightNumber);
                            break;
                        case Token::Condition::GE:
                            conditionIsTrue = (leftNumber >= rightNumber);
                            break;
                        }

                        ifContentAnalyzer(jt, it, conditionIsTrue);

                        --ifCount;
                        ifIt = end;
                    }
                }
            } else if (it->token.type() == Token::Type::CONDITION) {
                if (ifCount == 0)
                    throw CompileError("conditions without IF is illegal", it->pos);
            }

            ++it;
        }

        if (ifCount != 0)
            throw CompileError("unclosed IF", ifIt->pos);
    };

    ifContentAnalyzer = [&](ItType begin, ItType end, bool isConditionTrue) {
        ItType it = begin;
        size_t ifCount = 0;
        ItType elseIt = end;
        while (it != end) {
            
            if (it->token.type() == Token::Type::CONDITION_DIRECTIVE) {
                if (it->token.value<Token::ConditionDirective>() == Token::ConditionDirective::IF)
                    ++ifCount;
                else if (it->token.value<Token::ConditionDirective>() == Token::ConditionDirective::ELSE) {
                    if (ifCount == 0) {
                        elseIt = it;
                        break;
                    }
                } else {
                    if (ifCount > 0)
                        --ifCount;
                }
            }

            ++it;
        }

        if (elseIt == end) {
            if (isConditionTrue)
                ifAnalyzer(begin, end);
            else
                excludes.insert(excludes.end(), begin, end);
        } else {
            if (isConditionTrue) {
                excludes.insert(excludes.end(), elseIt, end);
                ifAnalyzer(begin, elseIt);
            } else {
                excludes.insert(excludes.end(), begin, elseIt + 1);
                ifAnalyzer(elseIt + 1, end);
            }
        }
    };

    ifAnalyzer(tokenContainerVector.begin(), tokenContainerVector.end());

    return excludeUsedTokens(tokenContainerVector, excludes);
}

auto processSymbolicConstantReplace(vector<TokenContainer> tokenContainerVector, const map<string, Integer> &equMap) {
    for (auto it = tokenContainerVector.begin(); it != tokenContainerVector.end(); ++it) {
        if (it->token.type() == Token::Type::USER_IDENTIFIER) {
            string identifier = it->token.value<string>();
            if (equMap.count(identifier))
                *it = {it->pos, Token(Token::Type::CONSTANT_NUMBER, equMap.find(identifier)->second)};
        }
    }

    return tokenContainerVector;
}

auto processSegmentsParting(const vector<TokenContainer> &tokenContainerVector) {
    vector<TokenSegment> segmentTokenContainerVector;

    vector<TokenContainer> excludes;

    auto segmentStartIt = tokenContainerVector.end();
    for (auto it = tokenContainerVector.begin(); it < tokenContainerVector.end(); ++it) {
        if (it->token.type() == Token::Type::SEGMENT_DIRECTIVE) {
            if (it->token.value<Token::SegmentDirective>() == Token::SegmentDirective::SEGMENT) {
                if (segmentStartIt == tokenContainerVector.end()) {
                    if ((it != tokenContainerVector.begin()) &&
                        ((it - 1)->token.type() == Token::Type::USER_IDENTIFIER))
                        segmentStartIt = it;
                    else
                        throw CompileError("SEGMENT must have a name", it->pos);
                } else
                    throw CompileError("you cannot declare a segment inside another one", it->pos);
            } else {
                if (segmentStartIt != tokenContainerVector.end()) {
                    if ((it != tokenContainerVector.begin()) &&
                        ((it - 1)->token.type() == Token::Type::USER_IDENTIFIER) &&
                        ((it - 1)->token.value<string>() == (segmentStartIt - 1)->token.value<string>()))
                    {
                        segmentTokenContainerVector.push_back({(it - 1)->token.value<string>(), vector<TokenContainer>(segmentStartIt + 1, it - 1)});

                        excludes.insert(excludes.end(), segmentStartIt - 1, it + 1);

                        segmentStartIt = tokenContainerVector.end();
                    }
                    else
                        throw CompileError("ENDS require a name", it->pos);
                } else
                    throw CompileError("ENDS must end a SEGMENT", it->pos);
            }
        }
    }

    vector<TokenContainer> remains = excludeUsedTokens(tokenContainerVector, excludes);

    if ((remains.end() - 1)->token.type() == Token::Type::END_DIRECTIVE)
        remains.erase(remains.end() - 1);
    else if (((remains.end() - 1)->token.type() == Token::Type::USER_IDENTIFIER) &&
             ((remains.end() - 2)->token.type() == Token::Type::END_DIRECTIVE))
        remains.erase(remains.end() - 2, remains.end());

    if (!remains.empty())
        throw CompileError("undefined expression outside segment", remains.begin()->pos);

    return segmentTokenContainerVector;
}

tuple<vector<TokenSegment>, map<string, Integer>> preprocess(const vector<TokenContainer> &tokenContainerVector) {
    auto equPhaseResult = processEQUs(tokenContainerVector);
    auto ifPhaseResult = processIFs(get<1>(equPhaseResult), get<0>(equPhaseResult));
    auto constantReplaceResult = processSymbolicConstantReplace(ifPhaseResult, get<0>(equPhaseResult));
    auto segmentsPartingResult = processSegmentsParting(constantReplaceResult);
    return make_tuple(segmentsPartingResult, get<0>(equPhaseResult));
}
