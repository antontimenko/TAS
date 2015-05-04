#ifndef _TSCOMPILER_H_
#define _TSCOMPILER_H_

#include "TSGlobal.h"

class TSCompiler
{
public:
    enum class Arch
    {
        X86_16,
        X86_32
    };

    void compile(const string &sourceFilePath, const string &resultFilePath) const;
    static TSCompiler &instance();

    static const Arch defaultArch;
private:
    TSCompiler();
    TSCompiler(const TSCompiler &) = delete;
    TSCompiler &operator=(const TSCompiler &) = delete;
};

void TSCompile(const string &sourceFilePath, const string &resultFilePath);

#endif
