#include "TSMath.h"

#include "TSException.h"

TSInteger mathExpressionComputer(const vector<TSMathOperation> &mathOperationVector)
{
    if (mathOperationVector.empty())
        return 0_I;

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
            throw TSCompileError("extra close bracket is illegal", it->pos);

        if ((bracketCount == 0) && 
            ((it != (mathOperationVector.end() - 1)) || 
             ((it == (mathOperationVector.end() - 1)) &&
              (it->kind != TSMathOperationKind::BRACKET_CLOSE))))
            isOperationWholeBracket = false;
    }

    if (bracketCount > 0)
        throw TSCompileError("unclosed bracket is illegal",
                             {(mathOperationVector.end() - 1)->pos.row,
                              (mathOperationVector.end() - 1)->pos.column + 1,
                              0});

    if (isOperationWholeBracket)
    {
        vector<TSMathOperation> newMathOperationVector(mathOperationVector.begin() + 1, mathOperationVector.end() - 1);
        
        if (newMathOperationVector.empty())
            throw TSCompileError("empty expression is illegal", mathOperationVector.begin()->pos);
        
        return mathExpressionComputer(newMathOperationVector);
    }
    else
    {
        if (operationIt->kind == TSMathOperationKind::ADD)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'+' must have right value", operationIt->pos);
            
            if (newMathOperationVectorLeft.empty())
                return mathExpressionComputer(newMathOperationVectorRight);
            else
            {
                try
                {
                    return mathExpressionComputer(newMathOperationVectorLeft) + mathExpressionComputer(newMathOperationVectorRight);
                }
                catch (std::overflow_error &e)
                {
                    throw TSCompileError(e.what(), operationIt->pos);
                }
            }
        }
        else if (operationIt->kind == TSMathOperationKind::SUBTRACT)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'-' must have right value", operationIt->pos);
            
            try
            {
                if (newMathOperationVectorLeft.empty())
                    return -mathExpressionComputer(newMathOperationVectorRight);
                else
                    return mathExpressionComputer(newMathOperationVectorLeft) - mathExpressionComputer(newMathOperationVectorRight);
            }
            catch (std::overflow_error &e)
            {
                throw TSCompileError(e.what(), operationIt->pos);
            }
        }
        else if (operationIt->kind == TSMathOperationKind::MULTIPLY)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'*' must have right value", operationIt->pos);

            if (newMathOperationVectorLeft.empty())
                throw TSCompileError("'*' cannot be unary", operationIt->pos);
            
            try
            {
                return mathExpressionComputer(newMathOperationVectorLeft) * mathExpressionComputer(newMathOperationVectorRight);
            }
            catch (std::overflow_error &e)
            {
                throw TSCompileError(e.what(), operationIt->pos);
            }
        }
        else if (operationIt->kind == TSMathOperationKind::DIVIDE)
        {
            vector<TSMathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<TSMathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw TSCompileError("'/' must have right value", operationIt->pos);

            if (newMathOperationVectorLeft.empty())
                throw TSCompileError("'/' cannot be unary", operationIt->pos);

            try
            {
                return mathExpressionComputer(newMathOperationVectorLeft) / mathExpressionComputer(newMathOperationVectorRight);
            }
            catch (std::overflow_error &e)
            {
                throw TSCompileError(e.what(), operationIt->pos);
            }
        }
        else if (operationIt->kind == TSMathOperationKind::CONSTANT)
        {
            return operationIt->value;
        }
        else
        {
            throw TSCompileError("illegal expression", operationIt->pos);
        }
    }
}
