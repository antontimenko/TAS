#include "TSCompiler.h"

#include "TSLexicalAnalyze.h"
#include <fstream>

TSCompiler::TSCompiler()
{
}

void TSCompiler::compile(const string &sourceFilePath, const string &resultFilePath) const
{
    std::ifstream sourceFile(sourceFilePath);

    if (sourceFile.is_open())
    {
        string sourceFileContents((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        vector<string> lexemeVector = constructLexemeVector(sourceFileContents);

        for (uint i = 0; i < lexemeVector.size(); ++i)
            cout << lexemeVector[i] << endl;
    }
    else
        cout << "Error: file not found, or permission denied" << endl;
}

TSCompiler &TSCompiler::instance()
{
    static TSCompiler compiler;
    return compiler;
}

void TSCompile(const string &sourceFilePath, const string &resultFilePath)
{
    TSCompiler::instance().compile(sourceFilePath, resultFilePath);
}
