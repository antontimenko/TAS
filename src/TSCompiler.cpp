#include "TSCompiler.h"

TSCompiler::TSCompiler()
{
}

TSCompiler &TSCompiler::instance()
{
    static TSCompiler compiler;
    return compiler;
}

void TSCompile(string &sourceFilePath, string &resultFilePath)
{
    
}
