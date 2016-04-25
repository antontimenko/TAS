#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "Global.h"

class Compiler {
public:
    enum class Arch
    {
        X86_16,
        X86_32
    };

    void compile(const string &sourceFilePath) const;
    static Compiler &instance();

    static const Arch arch;
private:
    Compiler();
    Compiler(const Compiler &) = delete;
    Compiler &operator=(const Compiler &) = delete;
};

void Compile(const string &sourceFilePath);

#endif
