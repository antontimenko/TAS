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
    TSCompileError(string text, uint row, uint column, uint length = 0);
    virtual const uint &row() const;
    virtual const uint &column() const;
    virtual const uint &length() const;
protected:
    const uint _row;
    const uint _column;
    const uint _length;
};

#endif
