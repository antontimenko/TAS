#include "TSMath.h"

#include "TSException.h"
#include "TSParser.h"

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
            throw TSCompileError("extra close bracket is illegal", *it);

        if ((bracketCount == 0) && 
            ((it != (mathOperationVector.end() - 1)) || 
             ((it == (mathOperationVector.end() - 1)) &&
              (it->kind != TSMathOperationKind::BRACKET_CLOSE))))
            isOperationWholeBracket = false;
    }

    if (bracketCount > 0)
        throw TSCompileError("unclosed bracket is illegal",
                             (mathOperationVector.end() - 1)->row,
                             (mathOperationVector.end() - 1)->column + 1,
                             0);

    if (isOperationWholeBracket)
    {
        vector<TSMathOperation> newMathOperationVector(mathOperationVector.begin() + 1, mathOperationVector.end() - 1);
        
        if (newMathOperationVector.empty())
            throw TSCompileError("empty expression is illegal", *mathOperationVector.begin());
        
        return mathExpressionComputer(newMathOperationVector);
    }
    else
    {
        if (operationIt->kind == TSMathOperationKind::ADD)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'+' must have right value", *operationIt);
            
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
                throw TSCompileError("'-' must have right value", *operationIt);
            
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
                throw TSCompileError("'*' must have right value", *operationIt);

            if (newMathOperationVectorLeft.empty())
                throw TSCompileError("'*' cannot be unary", *operationIt);
            
            return mathExpressionComputer(newMathOperationVectorLeft) * mathExpressionComputer(newMathOperationVectorRight);
        }
        else if (operationIt->kind == TSMathOperationKind::DIVIDE)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'/' must have right value", *operationIt);

            if (newMathOperationVectorLeft.empty())
                throw TSCompileError("'/' cannot be unary", *operationIt);

            if (mathExpressionComputer(newMathOperationVectorRight) == 0)
                throw TSCompileError("division by zero", *newMathOperationVectorRight.begin());
            
            return mathExpressionComputer(newMathOperationVectorLeft) / mathExpressionComputer(newMathOperationVectorRight);
        }
        else if (operationIt->kind == TSMathOperationKind::CONSTANT)
        {
            return operationIt->value;
        }
        else
        {
            throw TSCompileError("ilegal expression", *operationIt);
        }
    }
}
