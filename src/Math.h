#ifndef _MATH_H_
#define _MATH_H_

#include "Global.h"
#include "CodePosition.h"
#include "Integer.h"

enum MathOperationKind {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    CONSTANT,
    BRACKET_OPEN,
    BRACKET_CLOSE
};

struct MathOperation {
    MathOperationKind kind;
    Integer value;
    CodePosition pos;
};

Integer mathExpressionComputer(const vector<MathOperation> &mathOperationVector);

#endif
