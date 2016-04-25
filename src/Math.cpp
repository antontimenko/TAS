#include "Math.h"

#include "Exception.h"

Integer mathExpressionComputer(const vector<MathOperation> &mathOperationVector) {
    if (mathOperationVector.empty())
        return 0;

    auto operationIt = mathOperationVector.end();

    bool isOperationWholeBracket = true;
    int bracketCount = 0;
    for (auto it = mathOperationVector.begin(); it != mathOperationVector.end(); ++it) {
        if (bracketCount == 0) {
            if ((it->kind == MathOperationKind::ADD) ||
                (it->kind == MathOperationKind::SUBTRACT))
            {
                operationIt = it;
            } else if ((it->kind == MathOperationKind::MULTIPLY) ||
                     (it->kind == MathOperationKind::DIVIDE))
            {
                if ((operationIt == mathOperationVector.end()) ||
                    ((operationIt->kind != MathOperationKind::ADD) &&
                     (operationIt->kind != MathOperationKind::SUBTRACT)))
                {
                    operationIt = it;
                }
            } else if (it->kind == MathOperationKind::CONSTANT) {
                if (operationIt == mathOperationVector.end())
                    operationIt = it;
            } else if (it->kind == MathOperationKind::BRACKET_OPEN) {
                if ((operationIt == mathOperationVector.end()) ||
                    (operationIt->kind == MathOperationKind::CONSTANT) ||
                    (operationIt->kind == MathOperationKind::BRACKET_OPEN))
                {
                    operationIt = it;
                }
            }
        }

        if (it->kind == MathOperationKind::BRACKET_OPEN)
            ++bracketCount;
        else if (it->kind == MathOperationKind::BRACKET_CLOSE)
            --bracketCount;

        if (bracketCount < 0)
            throw CompileError("extra close bracket is illegal", it->pos);

        if ((bracketCount == 0) && 
            ((it != (mathOperationVector.end() - 1)) || 
             ((it == (mathOperationVector.end() - 1)) &&
              (it->kind != MathOperationKind::BRACKET_CLOSE))))
            isOperationWholeBracket = false;
    }

    if (bracketCount > 0)
        throw CompileError("unclosed bracket is illegal",
                             {(mathOperationVector.end() - 1)->pos.row,
                              (mathOperationVector.end() - 1)->pos.column + 1,
                              0});

    if (isOperationWholeBracket) {
        vector<MathOperation> newMathOperationVector(mathOperationVector.begin() + 1, mathOperationVector.end() - 1);
        
        if (newMathOperationVector.empty())
            throw CompileError("empty expression is illegal", mathOperationVector.begin()->pos);
        
        return mathExpressionComputer(newMathOperationVector);
    } else {
        if (operationIt->kind == MathOperationKind::ADD) {
            vector<MathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<MathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw CompileError("'+' must have right value", operationIt->pos);
            
            if (newMathOperationVectorLeft.empty())
                return mathExpressionComputer(newMathOperationVectorRight);
            else {
                try {
                    return mathExpressionComputer(newMathOperationVectorLeft) + mathExpressionComputer(newMathOperationVectorRight);
                } catch (std::overflow_error &e) {
                    throw CompileError(e.what(), operationIt->pos);
                }
            }
        } else if (operationIt->kind == MathOperationKind::SUBTRACT) {
            vector<MathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<MathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw CompileError("'-' must have right value", operationIt->pos);
            
            try {
                if (newMathOperationVectorLeft.empty())
                    return -mathExpressionComputer(newMathOperationVectorRight);
                else
                    return mathExpressionComputer(newMathOperationVectorLeft) - mathExpressionComputer(newMathOperationVectorRight);
            } catch (std::overflow_error &e) {
                throw CompileError(e.what(), operationIt->pos);
            }
        } else if (operationIt->kind == MathOperationKind::MULTIPLY) {
            vector<MathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<MathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw CompileError("'*' must have right value", operationIt->pos);

            if (newMathOperationVectorLeft.empty())
                throw CompileError("'*' cannot be unary", operationIt->pos);
            
            try {
                return mathExpressionComputer(newMathOperationVectorLeft) * mathExpressionComputer(newMathOperationVectorRight);
            } catch (std::overflow_error &e) {
                throw CompileError(e.what(), operationIt->pos);
            }
        } else if (operationIt->kind == MathOperationKind::DIVIDE) {
            vector<MathOperation> newMathOperationVectorLeft(mathOperationVector.begin(), operationIt);
            vector<MathOperation> newMathOperationVectorRight(operationIt + 1, mathOperationVector.end());

            if (newMathOperationVectorRight.empty())
                throw CompileError("'/' must have right value", operationIt->pos);

            if (newMathOperationVectorLeft.empty())
                throw CompileError("'/' cannot be unary", operationIt->pos);

            try {
                return mathExpressionComputer(newMathOperationVectorLeft) / mathExpressionComputer(newMathOperationVectorRight);
            } catch (std::overflow_error &e) {
                throw CompileError(e.what(), operationIt->pos);
            }
        } else if (operationIt->kind == MathOperationKind::CONSTANT)
            return operationIt->value;
        else
            throw CompileError("illegal expression", operationIt->pos);
    }
}
