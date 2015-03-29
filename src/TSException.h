#ifndef _TSEXCEPTION_H_
#define _TSEXCEPTION_H_

#include "TSGlobal.h"
#include "TSToken.h"
#include "TSMath.h"
#include <exception>

class TSException : public std::exception
{
public:
    TSException(string text);
    virtual const char *what() const noexcept;
protected:
    string text;
};

class TSCompileError : public TSException
{
public: 
    TSCompileError(string text, size_t row, size_t column, size_t length = 0);
    TSCompileError(string text, TSTokenContainer tokenContainer);
    TSCompileError(string text, TSMathOperation mathOperation);
    virtual const size_t &row() const;
    virtual const size_t &column() const;
    virtual const size_t &length() const;
protected:
    const size_t _row;
    const size_t _column;
    const size_t _length;
};

#endif
