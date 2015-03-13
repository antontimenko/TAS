#ifndef _TSEXCEPTION_H_
#define _TSEXCEPTION_H_

#include "TSGlobal.h"
#include <exception>

class TSException : public std::exception
{
public:
    TSException() = delete;
    TSException(string text);
    virtual const char *what() const noexcept;
protected:
    string text;
};

class TSCompileError : public TSException
{
public:
    TSCompileError() = delete;
    TSCompileError(string text, size_t row, size_t column, size_t length = 0);
    virtual const size_t &row() const;
    virtual const size_t &column() const;
    virtual const size_t &length() const;
protected:
    const size_t _row;
    const size_t _column;
    const size_t _length;
};

#endif
