#include "TSCompiler.h"

#include "TSLexicalAnalyze.h"
#include "TSException.h"
#include "TSUtility.h"
#include "TSToken.h"
#include "TSDiagnostics.h"
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

            vector<TSTokenContainer> tokenContainerVector = constructTokenContainerVector(lexemeContainerVector);

            for (uint i = 0; i < tokenContainerVector.size(); ++i)
            {
                cout << "(" << tokenContainerVector[i].row << "," << tokenContainerVector[i].column << "): ";
                
                switch (tokenContainerVector[i].token.getType())
                {
                    case TSToken::Type::userIdentifier:
                        cout << "userIdentifier: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::singleChar:
                        cout << "singleChar: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::directive:
                        cout << "directive: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::instruction:
                        cout << "instruction: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::register8:
                        cout << "register8: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::register32:
                        cout << "register32: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::registerSegment:
                        cout << "registerSegment: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::sizeOperator:
                        cout << "sizeOperator: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::sizeIdentifier:
                        cout << "sizeIdentifier: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::dataIdentifier:
                        cout << "dataIdentifier: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::constantNumber:
                        cout << "constantNumber: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::constantString:
                        cout << "constantString: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                    case TSToken::Type::condition:
                        cout << "condition: " << lexemeContainerVector[i].lexeme << endl;
                        break;
                }
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
