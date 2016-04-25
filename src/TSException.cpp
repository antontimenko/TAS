#include "TSException.h"

TSException::TSException(string text) :
    text(text)
{}

const char *TSException::what() const noexcept {
    return text.c_str();
}

TSCompileError::TSCompileError(string text, TSCodePosition pos) :
    TSException(text),
    _pos(pos)
{}

const TSCodePosition &TSCompileError::pos() const {
    return _pos;
}
