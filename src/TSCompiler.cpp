#include "TSCompiler.h"

#include "TSLexeme.h"
#include "TSException.h"
#include "TSToken.h"
#include "TSDiagnostics.h"
#include "TSPreprocessor.h"
#include "TSPseudoSentence.h"
#include "TSRawSentence.h"
#include "TSSentence.h"
#include <fstream>

const TSCompiler::Arch TSCompiler::arch = Arch::X86_32;

TSCompiler::TSCompiler()
{}

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
            auto phase1 = constructLexemeContainerVector(sourceFileContents);
            auto phase2 = convertLexemeContainerVectorToUpperCase(phase1);
            auto phase3 = constructTokenContainerVector(phase2);
            auto phase4 = preprocess(phase3);
            auto phase5 = splitPseudoSentences(phase4);
            auto phase6 = constructRawSentences(get<0>(phase5), get<1>(phase5));
            auto phase7 = constructSentences(phase6);
            printListing(phase7);
        }
        catch (TSCompileError &e)
        {
            printCompileError(e.what(), sourceFileContents, e.pos());
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
