#include "TSCompiler.h"

#include "TSLexeme.h"
#include "TSException.h"
#include "TSToken.h"
#include "TSDiagnostics.h"
#include "TSParser.h"
#include <fstream>

TSCompiler::TSCompiler()
{
}

void TSCompiler::compile(const string &sourceFilePath, const string &resultFilePath) const
{
    try
    {
        std::ifstream sourceFile(sourceFilePath);
        if (!sourceFile.is_open())
            throw TSException(string("File \'") + sourceFilePath + "\' not found, or permission denied");

        string sourceFileContents((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        try
        {
            vector<TSLexemeContainer> lexemeContainerVector = constructLexemeContainerVector(sourceFileContents);
            lexemeContainerVector = convertLexemeContainerVectorToUpperCase(lexemeContainerVector);

            vector<TSTokenContainer> tokenContainerVector = TSToken::constructTokenContainerVector(lexemeContainerVector);

            //printLexemeTable(lexemeContainerVector, tokenContainerVector);

            parse(tokenContainerVector);
        }
        catch (TSCompileError &e)
        {
            printCompileError(e.what(), sourceFileContents, e.row(), e.column(), e.length());
        }
    }
    catch (std::exception &e)
    {
        printError(e.what());
    }
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
