#include "TSDiagnostics.h"

#include "TSUtility.h"
#include <stdio.h>

const char *TSColor::Reset = "\033[0m";
const char *TSColor::Black = "\033[30m";
const char *TSColor::Red = "\033[31m";
const char *TSColor::Green = "\033[32m";
const char *TSColor::Yellow = "\033[33m";
const char *TSColor::Blue = "\033[34m";
const char *TSColor::Magenta = "\033[35m";
const char *TSColor::Cyan = "\033[36m";
const char *TSColor::White = "\033[37m";
const char *TSColor::BBlack = "\033[1m\033[30m";
const char *TSColor::BRed = "\033[1m\033[31m";
const char *TSColor::BGreen = "\033[1m\033[32m";
const char *TSColor::BYellow = "\033[1m\033[33m";
const char *TSColor::BBlue = "\033[1m\033[34m";
const char *TSColor::BMagenta = "\033[1m\033[35m";
const char *TSColor::BCyan = "\033[1m\033[36m";
const char *TSColor::BWhite = "\033[1m\033[37m";

void printError(string text)
{
    cout << TSColor::BWhite << text << TSColor::Reset << endl;
}

void printCompileError(string text, const string &sourceFileContents, size_t row, size_t column, size_t length)
{
    cout << TSColor::BWhite << flush;

    cout << TSColor::BRed << "Compile Error" << TSColor::BWhite << " ("
         << row << ":"
         << column << "): "
         << text << endl;

    size_t i;
    size_t j;

    i = 0;
    size_t currentRow = 1;
    while (currentRow < row)
    {
        if ((sourceFileContents[i] == cCR) || (sourceFileContents[i] == cLF))
            ++currentRow;
        if ((sourceFileContents[i] == cCR) && (sourceFileContents[i + 1] == cLF))
            ++i;
        ++i;
    }

    size_t lineStartIndex = i;
    
    j = 1;
    while ((sourceFileContents[i] != cCR) && (sourceFileContents[i] != cLF) && (i < sourceFileContents.size()))
    {
        if (j == column)
            cout << TSColor::BRed << flush;
        else if (j == column + length)
            cout << TSColor::BWhite << flush;

        if (sourceFileContents[i] == 0x9)
            cout << "    ";
        else
            cout << sourceFileContents[i];

        ++i;
        ++j;
    }
    cout << endl;

    i = lineStartIndex;
    j = 1;
    while (j < column)
    {
        if (sourceFileContents[i] == 0x9)
            cout << "    ";
        else
            cout << " ";

        ++i;
        ++j;
    }
    cout << TSColor::BGreen << "^" << TSColor::BWhite << endl;

    cout << TSColor::Reset << flush;
}

void printLexemeTable(const vector<TSLexemeContainer> &lexemeContainerVector, const vector<TSTokenContainer> &tokenContainerVector)
{
    cout << TSColor::BWhite << "---LEXEME TABLE OUTPUT---" << TSColor::Reset << endl << endl;

    vector<string> coordsStringVector;
    size_t maxCoordsSize = 0;
    for (size_t i = 0; i < tokenContainerVector.size(); ++i)
    {
        coordsStringVector.push_back((string("(") + std::to_string(tokenContainerVector[i].row) + "," + std::to_string(tokenContainerVector[i].column) + ")"));
        if (coordsStringVector[i].size() > maxCoordsSize)
            maxCoordsSize = coordsStringVector[i].size();
    }
    
    for (size_t i = 0; i < tokenContainerVector.size(); ++i)
    {
        printf("%*s: ", maxCoordsSize, coordsStringVector[i].c_str());

        switch (tokenContainerVector[i].token.type())
        {
            case TSToken::Type::userIdentifier:
                printf("%15s | %s\n", "userIdentifier", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::singleChar:
                printf("%15s | %s\n", "singleChar", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::directive:
                printf("%15s | %s\n", "directive", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::instruction:
                printf("%15s | %s\n", "instruction", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::register8:
                printf("%15s | %s\n", "register8", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::register32:
                printf("%15s | %s\n", "register32", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::registerSegment:
                printf("%15s | %s\n", "registerSegment", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::sizeOperator:
                printf("%15s | %s\n", "sizeOperator", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::sizeIdentifier:
                printf("%15s | %s\n", "sizeIdentifier", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::dataIdentifier:
                printf("%15s | %s\n", "dataIdentifier", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::constantNumber:
                printf("%15s | %s\n", "constantNumber", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::constantString:
                printf("%15s | %s\n", "constantString", lexemeContainerVector[i].lexeme.c_str());
                break;
            case TSToken::Type::condition:
                printf("%15s | %s\n", "condition", lexemeContainerVector[i].lexeme.c_str());
                break;
            default:
                printf("%15s | %s\n", "null", lexemeContainerVector[i].lexeme.c_str());
                break;
        }
    }

    cout << endl;
}
