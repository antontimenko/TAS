#ifndef _TSMATH_H_
#define _TSMATH_H_

#include "TSGlobal.h"
#include "TSCodePosition.h"
#include "TSInteger.h"

enum TSMathOperationKind {
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    CONSTANT,
    BRACKET_OPEN,
    BRACKET_CLOSE
};

struct TSMathOperation {
    TSMathOperationKind kind;
    TSInteger value;
    TSCodePosition pos;
};

TSInteger mathExpressionComputer(const vector<TSMathOperation> &mathOperationVector);

#endif
