#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include "Global.h"
#include "CodePosition.h"
#include "Token.h"
#include "Math.h"
#include <exception>

class Exception : public std::exception {
public:
    Exception(string text);
    virtual const char *what() const noexcept;
protected:
    string text;
};

class CompileError : public Exception {
public: 
    CompileError(string text, CodePosition pos);
    virtual const CodePosition &pos() const;
protected:
    const CodePosition _pos;
};

#endif
