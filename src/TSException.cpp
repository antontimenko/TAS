#include "TSException.h"

TSException::TSException(string text) :
    text(text)
{
}

const char *TSException::what() const noexcept
{
    return text.c_str();
}

TSCompileError::TSCompileError(string text, size_t row, size_t column, size_t length) :
    TSException(text),
    _row(row),
    _column(column),
    _length(length)
{
}

const size_t &TSCompileError::row() const
{
    return _row;
}

const size_t &TSCompileError::column() const
{
    return _column;
}

const size_t &TSCompileError::length() const
{
    return _length;
}
