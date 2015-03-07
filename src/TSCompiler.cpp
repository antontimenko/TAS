#include "TSCompiler.h"

#include <fstream>

TSCompiler::TSCompiler()
{
}

void TSCompiler::compile(string &sourceFilePath, string &resultFilePath)
{
    std::ifstream sourceFile(sourceFilePath);

    if (sourceFile.is_open())
    {
        string sourceFileContents((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        cout << sourceFileContents << endl;
    }
    else
        cout << "Error: file not found, or permission denied" << endl;
}

TSCompiler &TSCompiler::instance()
{
    static TSCompiler compiler;
    return compiler;
}

void TSCompile(string &sourceFilePath, string &resultFilePath)
{
    TSCompiler::instance().compile(sourceFilePath, resultFilePath);
}
