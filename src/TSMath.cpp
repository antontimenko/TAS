#include "TSMath.h"

#include "TSException.h"
#include "TSParser.h"

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

longlong mathExpressionComputer(vector<TSMathOperation> mathOperationVector)
{
    auto operationIt = mathOperationVector.end();

    bool isOperationWholeBracket = true;
    int bracketCount = 0;
    for (auto it = mathOperationVector.begin(); it != mathOperationVector.end(); ++it)
    {
        if (bracketCount == 0)
        {
            if ((it->kind == TSMathOperationKind::ADD) ||
                (it->kind == TSMathOperationKind::SUBTRACT))
            {
                operationIt = it;
            }
            else if ((it->kind == TSMathOperationKind::MULTIPLY) ||
                     (it->kind == TSMathOperationKind::DIVIDE))
            {
                if ((operationIt == mathOperationVector.end()) ||
                    ((operationIt->kind != TSMathOperationKind::ADD) &&
                     (operationIt->kind != TSMathOperationKind::SUBTRACT)))
                {
                    operationIt = it;
                }
            }
            else if (it->kind == TSMathOperationKind::CONSTANT)
            {
                if (operationIt == mathOperationVector.end())
                    operationIt = it;
            }
            else if (it->kind == TSMathOperationKind::BRACKET_OPEN)
            {
                if ((operationIt == mathOperationVector.end()) ||
                    (operationIt->kind == TSMathOperationKind::CONSTANT) ||
                    (operationIt->kind == TSMathOperationKind::BRACKET_OPEN))
                {
                    operationIt = it;
                }
            }
        }

        if (it->kind == TSMathOperationKind::BRACKET_OPEN)
            ++bracketCount;
        else if (it->kind == TSMathOperationKind::BRACKET_CLOSE)
            --bracketCount;

        if (bracketCount < 0)
            throw TSCompileError("extra close bracket",
                                 it->row,
                                 it->column,
                                 it->length);

        

        if ((bracketCount == 0) && 
            ((it != (mathOperationVector.end() - 1)) || 
             ((it == (mathOperationVector.end() - 1)) &&
              (it->kind != TSMathOperationKind::BRACKET_CLOSE))))
            isOperationWholeBracket = false;
    }

    if (bracketCount > 0)
        throw TSCompileError("unclosed bracket",
                             (mathOperationVector.end() - 1)->row,
                             (mathOperationVector.end() - 1)->column + 1,
                             0);

    if (isOperationWholeBracket)
    {
        vector<TSMathOperation> newMathOperationVector(mathOperationVector.begin() + 1, mathOperationVector.end() - 1);
        
        if (newMathOperationVector.empty())
            throw TSCompileError("empty expression",
                                 mathOperationVector.begin()->row,
                                 mathOperationVector.begin()->column,
                                 mathOperationVector.begin()->length);
        
        return mathExpressionComputer(newMathOperationVector);
    }
    else
    {
        if (operationIt->kind == TSMathOperationKind::ADD)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'+' must have right value",
                                     operationIt->row,
                                     operationIt->column,
                                     operationIt->length);
            
            if (newMathOperationVectorLeft.empty())
                return mathExpressionComputer(newMathOperationVectorRight);
            else
                return mathExpressionComputer(newMathOperationVectorLeft) + mathExpressionComputer(newMathOperationVectorRight);
        }
        else if (operationIt->kind == TSMathOperationKind::SUBTRACT)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'-' must have right value",
                                     operationIt->row,
                                     operationIt->column,
                                     operationIt->length);
            
            if (newMathOperationVectorLeft.empty())
                return -mathExpressionComputer(newMathOperationVectorRight);
            else
                return mathExpressionComputer(newMathOperationVectorLeft) - mathExpressionComputer(newMathOperationVectorRight);
        }
        else if (operationIt->kind == TSMathOperationKind::MULTIPLY)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'*' must have right value",
                                     operationIt->row,
                                     operationIt->column,
                                     operationIt->length);

            if (newMathOperationVectorLeft.empty())
                throw TSCompileError("'*' cannot be unary",
                                     operationIt->row,
                                     operationIt->column,
                                     operationIt->length);
            
            return mathExpressionComputer(newMathOperationVectorLeft) * mathExpressionComputer(newMathOperationVectorRight);
        }
        else if (operationIt->kind == TSMathOperationKind::DIVIDE)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'/' must have right value",
                                     operationIt->row,
                                     operationIt->column,
                                     operationIt->length);

            if (newMathOperationVectorLeft.empty())
                throw TSCompileError("'/' cannot be unary",
                                     operationIt->row,
                                     operationIt->column,
                                     operationIt->length);

            if (mathExpressionComputer(newMathOperationVectorRight) == 0)
                throw TSCompileError("division by zero",
                                     newMathOperationVectorRight.begin()->row,
                                     newMathOperationVectorRight.begin()->column,
                                     newMathOperationVectorRight.begin()->length);
            
            return mathExpressionComputer(newMathOperationVectorLeft) / mathExpressionComputer(newMathOperationVectorRight);
        }
        else if (operationIt->kind == TSMathOperationKind::CONSTANT)
        {
            return operationIt->value;
        }
        else
        {
            throw TSCompileError("it cannot be like that, asshole",
                                 operationIt->row,
                                 operationIt->column,
                                 operationIt->length);
        }
    }
}
