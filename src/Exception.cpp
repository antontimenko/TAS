#include "Exception.h"

Exception::Exception(string text) :
    text(text)
{}

const char *Exception::what() const noexcept {
    return text.c_str();
}

CompileError::CompileError(string text, CodePosition pos) :
    Exception(text),
    _pos(pos)
{}

const CodePosition &CompileError::pos() const {
    return _pos;
}
