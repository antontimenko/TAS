#include "TSDiagnostics.h"

#include "TSUtility.h"

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

void printCompileError(string text, const string &sourceFileContents, uint row, uint column, uint length)
{
    cout << TSColor::BWhite << flush;

    cout << TSColor::BRed << "Compile Error" << TSColor::BWhite << " ("
         << row << ":"
         << column << "): "
         << text << endl;

    uint i;
    uint j;

    i = 0;
    uint currentRow = 1;
    while (currentRow < row)
    {
        if ((sourceFileContents[i] == cCR) || (sourceFileContents[i] == cLF))
            ++currentRow;
        if ((sourceFileContents[i] == cCR) && (sourceFileContents[i + 1] == cLF))
            ++i;
        ++i;
    }

    uint lineStartIndex = i;
    
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
