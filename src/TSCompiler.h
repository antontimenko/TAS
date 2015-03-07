#ifndef _TSCOMPILER_H_
#define _TSCOMPILER_H_

#include "TSGlobal.h"

class TSCompiler
{
public:
    void compile(string &sourceFilePath, string &resultFilePath);
    static TSCompiler &instance();
private:
    TSCompiler();
    TSCompiler(const TSCompiler &) = delete;
    TSCompiler &operator=(const TSCompiler &) = delete;
};

void TSCompile(string &sourceFilePath, string &resultFilePath);

#endif
