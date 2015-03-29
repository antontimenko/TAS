#include "TSDiagnostics.h"

#include <utility>
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

template<typename T, typename U>
typename map<T, U>::iterator findByValue(map<T, U> source, U value)
{
    for (auto it = source.begin(); it != source.end(); ++it)
    {
        if (value == it->second)
            return it;
    }

    return source.end();
}

void printTokenTable(const vector<TSTokenContainer> &tokenContainerVector)
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
        printf("%-4u | %-*s | ", i, maxCoordsSize, coordsStringVector[i].c_str());

        const TSToken &token = tokenContainerVector[i].token;

        switch (token.type())
        {
        case TSToken::Type::userIdentifier:
            printf("%-20s | %s\n", "userIdentifier", token.value<string>().c_str());
            break;
        case TSToken::Type::singleChar:
            printf("%-20s | %s\n", "singleChar", findByValue(TSToken::singleCharMap, token.value<TSToken::SingleChar>())->first.c_str());
            break;
        case TSToken::Type::mathSymbol:
            printf("%-20s | %s\n", "mathSymbol", findByValue(TSToken::mathSymbolMap, token.value<TSToken::MathSymbol>())->first.c_str());
            break;
        case TSToken::Type::directive:
            printf("%-20s | %s\n", "directive", findByValue(TSToken::directiveMap, token.value<TSToken::Directive>())->first.c_str());
            break;
        case TSToken::Type::instruction:
            printf("%-20s | %s\n", "instruction", findByValue(TSToken::instructionMap, token.value<TSToken::Instruction>())->first.c_str());
            break;
        case TSToken::Type::register8:
            printf("%-20s | %s\n", "register8", findByValue(TSToken::register8Map, token.value<TSToken::Register8>())->first.c_str());
            break;
        case TSToken::Type::register32:
            printf("%-20s | %s\n", "register32", findByValue(TSToken::register32Map, token.value<TSToken::Register32>())->first.c_str());
            break;
        case TSToken::Type::registerSegment:
            printf("%-20s | %s\n", "registerSegment", findByValue(TSToken::registerSegmentMap, token.value<TSToken::RegisterSegment>())->first.c_str());
            break;
        case TSToken::Type::sizeOperator:
            printf("%-20s | %s\n", "sizeOperator", TSToken::sizeOperatorStr.c_str());
            break;
        case TSToken::Type::sizeIdentifier:
            printf("%-20s | %s\n", "sizeIdentifier", findByValue(TSToken::sizeIdentifierMap, token.value<TSToken::SizeIdentifier>())->first.c_str());
            break;
        case TSToken::Type::dataIdentifier:
            printf("%-20s | %s\n", "dataIdentifier", findByValue(TSToken::dataIdentifierMap, token.value<TSToken::DataIdentifier>())->first.c_str());
            break;
        case TSToken::Type::constantNumber:
            printf("%-20s | %lli\n", "constantNumber", token.value<longlong>());
            break;
        case TSToken::Type::constantString:
            printf("%-20s | %s\n", "constantString", token.value<string>().c_str());
            break;
        case TSToken::Type::conditionDirective:
            printf("%-20s | %s\n", "conditionDirective", findByValue(TSToken::conditionDirectiveMap, token.value<TSToken::ConditionDirective>())->first.c_str());
            break;
        case TSToken::Type::condition:
            printf("%-20s | %s\n", "condition", findByValue(TSToken::conditionMap, token.value<TSToken::Condition>())->first.c_str());
            break;
        default:
            printf("%-20s | %s\n", "null", "NULL");
            break;
        }
    }

    cout << endl;
}

void printTokenTable(const vector<TSTokenContainer> &tokenContainerVector, const vector<TSLexemeContainer> &lexemeContainerVector)
{
    cout << TSColor::BWhite << "---NATIVE LEXEME TABLE OUTPUT---" << TSColor::Reset << endl << endl;

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
        printf("%-4u | %-*s | ", i, maxCoordsSize, coordsStringVector[i].c_str());

        const string &lexeme = lexemeContainerVector[i].lexeme;

        switch (tokenContainerVector[i].token.type())
        {
        case TSToken::Type::userIdentifier:
            printf("%-20s | %s\n", "userIdentifier", lexeme.c_str());
            break;
        case TSToken::Type::singleChar:
            printf("%-20s | %s\n", "singleChar", lexeme.c_str());
            break;
        case TSToken::Type::mathSymbol:
            printf("%-20s | %s\n", "mathSymbol", lexeme.c_str());
            break;
        case TSToken::Type::directive:
            printf("%-20s | %s\n", "directive", lexeme.c_str());
            break;
        case TSToken::Type::instruction:
            printf("%-20s | %s\n", "instruction", lexeme.c_str());
            break;
        case TSToken::Type::register8:
            printf("%-20s | %s\n", "register8", lexeme.c_str());
            break;
        case TSToken::Type::register32:
            printf("%-20s | %s\n", "register32", lexeme.c_str());
            break;
        case TSToken::Type::registerSegment:
            printf("%-20s | %s\n", "registerSegment", lexeme.c_str());
            break;
        case TSToken::Type::sizeOperator:
            printf("%-20s | %s\n", "sizeOperator", lexeme.c_str());
            break;
        case TSToken::Type::sizeIdentifier:
            printf("%-20s | %s\n", "sizeIdentifier", lexeme.c_str());
            break;
        case TSToken::Type::dataIdentifier:
            printf("%-20s | %s\n", "dataIdentifier", lexeme.c_str());
            break;
        case TSToken::Type::constantNumber:
            printf("%-20s | %s\n", "constantNumber", lexeme.c_str());
            break;
        case TSToken::Type::constantString:
            printf("%-20s | %s\n", "constantString", lexeme.c_str());
            break;
        case TSToken::Type::conditionDirective:
            printf("%-20s | %s\n", "conditionDirective", lexeme.c_str());
            break;
        case TSToken::Type::condition:
            printf("%-20s | %s\n", "condition", lexeme.c_str());
            break;
        default:
            printf("%-20s | %s\n", "null", lexeme.c_str());
            break;
        }
    }

    cout << endl;
}

void printEquTable(const map<string, longlong> &equMap)
{
    cout << TSColor::BWhite << "---EQU DIRECTIVE TABLE OUTPUT---" << TSColor::Reset << endl << endl;

    for (auto it = equMap.begin(); it != equMap.end(); ++it)
        printf("%20s | %lli\n", it->first.c_str(), it->second);

    cout << endl;
}
