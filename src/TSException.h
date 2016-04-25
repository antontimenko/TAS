#ifndef _TSEXCEPTION_H_
#define _TSEXCEPTION_H_

#include "TSGlobal.h"
#include "TSCodePosition.h"
#include "TSToken.h"
#include "TSMath.h"
#include <exception>

class TSException : public std::exception {
public:
    TSException(string text);
    virtual const char *what() const noexcept;
protected:
    string text;
};

class TSCompileError : public TSException {
public: 
    TSCompileError(string text, TSCodePosition pos);
    virtual const TSCodePosition &pos() const;
protected:
    const TSCodePosition _pos;
};

#endif
