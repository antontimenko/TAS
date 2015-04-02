#include "TSCompiler.h"

#include "TSLexeme.h"
#include "TSException.h"
#include "TSToken.h"
#include "TSDiagnostics.h"
#include "TSPreprocessor.h"
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
            vector<TSLexemeContainer> lexemeContainerVectorUncased = constructLexemeContainerVector(sourceFileContents);
            vector<TSLexemeContainer> lexemeContainerVector = convertLexemeContainerVectorToUpperCase(lexemeContainerVectorUncased);
            vector<TSTokenContainer> tokenContainerVector = TSToken::constructTokenContainerVector(lexemeContainerVector);

            vector<TSSegmentContainer> segmentContainerVector = preprocess(tokenContainerVector);

            for (auto it = segmentContainerVector.begin(); it != segmentContainerVector.end(); ++it)
            {
                cout << "Segment " << it->name << endl;
                printTokenTable(it->tokenContainerVector);
            }
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
