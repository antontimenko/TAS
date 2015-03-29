#ifndef _TSMATH_H_
#define _TSMATH_H_

#include "TSGlobal.h"

enum TSMathOperationKind
{
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    CONSTANT,
    BRACKET_OPEN,
    BRACKET_CLOSE
};

struct TSMathOperation
{
    TSMathOperationKind kind;
    longlong value;
    size_t row;
    size_t column;
    size_t length;
};

longlong mathExpressionComputer(vector<TSMathOperation> mathContainerVector);

#endif
