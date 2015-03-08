#include "TSCompiler.h"

#include "TSLexicalAnalyze.h"
#include "TSException.h"
#include "TSUtility.h"
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
            throw TSException(string("File \"") + sourceFilePath + "\" not found, or permission denied");

        string sourceFileContents((std::istreambuf_iterator<char>(sourceFile)), std::istreambuf_iterator<char>());

        vector<TSLexemeContainer> lexemeContainerVector = constructLexemeContainerVector(sourceFileContents);

        for (uint i = 0; i < lexemeContainerVector.size(); ++i)
            cout << "(" << lexemeContainerVector[i].row << ","
                 << lexemeContainerVector[i].column << "): "
                 << lexemeContainerVector[i].lexeme << endl;
    }
    catch (TSCompileError &e)
    {
        cout << "Compile Error: ("
             << e.row() << ","
             << e.column() << "): "
             << e.what() << endl;

        const string &sourceFileContents = e.sourceFileContents();
        uint i;

        i = 0;
        uint row = 1;
        while (row < e.row())
        {
            if ((sourceFileContents[i] == cCR) || (sourceFileContents[i] == cLF))
                ++row;
            if ((sourceFileContents[i] == cCR) && (sourceFileContents[i + 1] == cLF))
                ++i;
            ++i;
        }

        uint lineStartIndex = i;
        
        while ((sourceFileContents[i] != cCR) && (sourceFileContents[i] != cLF) && (i < sourceFileContents.size()))
        {
            if (sourceFileContents[i] == 0x9)
                cout << "    ";
            else
                cout << sourceFileContents[i];

            ++i;
        }
        cout << endl;

        i = lineStartIndex;
        uint j = 1;
        while (j < e.column())
        {
            if (sourceFileContents[i] == 0x9)
                cout << "    ";
            else
                cout << " ";

            ++i;
            ++j;
        }
        cout << "^" << endl;
    }
    catch (TSException &e)
    {
        cout << e.what() << endl;
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
