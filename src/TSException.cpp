#include "TSException.h"

TSException::TSException(string text) :
    text(text)
{
}

const char *TSException::what() const noexcept
{
    return text.c_str();
}

TSCompileError::TSCompileError(string text, uint row, uint column, uint length) :
    TSException(text),
    _row(row),
    _column(column),
    _length(length)
{
}

const uint &TSCompileError::row() const
{
    return _row;
}

const uint &TSCompileError::column() const
{
    return _column;
}

const uint &TSCompileError::length() const
{
    return _length;
}
