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
        case TSToken::Type::USER_IDENTIFIER:
            printf("%-20s | %s\n", "User Identifier", token.value<string>().c_str());
            break;
        case TSToken::Type::MEMORY_BRACKET:
            printf("%-20s | %s\n", "Memory Bracket", findByValue(TSToken::memoryBracketMap, token.value<TSToken::MemoryBracket>())->first.c_str());
            break;
        case TSToken::Type::MATH_SYMBOL:
            printf("%-20s | %s\n", "Math Symbol", findByValue(TSToken::mathSymbolMap, token.value<TSToken::MathSymbol>())->first.c_str());
            break;
        case TSToken::Type::SEGMENT_DIRECTIVE:
            printf("%-20s | %s\n", "Segment Directive", findByValue(TSToken::segmentDirectiveMap, token.value<TSToken::SegmentDirective>())->first.c_str());
            break;
        case TSToken::Type::INSTRUCTION:
            printf("%-20s | %s\n", "Instruction", findByValue(TSToken::instructionMap, token.value<TSToken::Instruction>())->first.c_str());
            break;
        case TSToken::Type::REGISTER_8:
            printf("%-20s | %s\n", "Register 8", findByValue(TSToken::register8Map, token.value<TSToken::Register8>())->first.c_str());
            break;
        case TSToken::Type::REGISTER_32:
            printf("%-20s | %s\n", "Register 32", findByValue(TSToken::register32Map, token.value<TSToken::Register32>())->first.c_str());
            break;
        case TSToken::Type::REGISTER_SEGMENT:
            printf("%-20s | %s\n", "Register Segment", findByValue(TSToken::registerSegmentMap, token.value<TSToken::RegisterSegment>())->first.c_str());
            break;
        case TSToken::Type::SIZE_IDENTIFIER:
            printf("%-20s | %s\n", "Size Identifier", findByValue(TSToken::sizeIdentifierMap, token.value<TSToken::SizeIdentifier>())->first.c_str());
            break;
        case TSToken::Type::DATA_IDENTIFIER:
            printf("%-20s | %s\n", "Data Identifier", findByValue(TSToken::dataIdentifierMap, token.value<TSToken::DataIdentifier>())->first.c_str());
            break;
        case TSToken::Type::CONSTANT_NUMBER:
            printf("%-20s | %lli\n", "Constant Number", token.value<longlong>());
            break;
        case TSToken::Type::CONSTANT_STRING:
            printf("%-20s | %s\n", "Constant String", token.value<string>().c_str());
            break;
        case TSToken::Type::CONDITION_DIRECTIVE:
            printf("%-20s | %s\n", "Condition Directive", findByValue(TSToken::conditionDirectiveMap, token.value<TSToken::ConditionDirective>())->first.c_str());
            break;
        case TSToken::Type::CONDITION:
            printf("%-20s | %s\n", "Condition", findByValue(TSToken::conditionMap, token.value<TSToken::Condition>())->first.c_str());
            break;
        case TSToken::Type::COMMA:
            printf("%-20s | %s\n", "Comma", TSToken::commaStr.c_str());
            break;
        case TSToken::Type::COLON:
            printf("%-20s | %s\n", "Colon", TSToken::colonStr.c_str());
            break;
        case TSToken::Type::SIZE_OPERATOR:
            printf("%-20s | %s\n", "Size Operator", TSToken::sizeOperatorStr.c_str());
            break;
        case TSToken::Type::EQU_DIRECTIVE:
            printf("%-20s | %s\n", "EQU Directive", TSToken::equDirectiveStr.c_str());
            break;
        case TSToken::Type::END_DIRECTIVE:
            printf("%-20s | %s\n", "END Directive", TSToken::endDirectiveStr.c_str());
            break;
        default:
            printf("%-20s | %s\n", "Undefined", "Undefined");
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
        case TSToken::Type::USER_IDENTIFIER:
            printf("%-20s | %s\n", "User Identifier", lexeme.c_str());
            break;
        case TSToken::Type::MEMORY_BRACKET:
            printf("%-20s | %s\n", "Memory Bracket", lexeme.c_str());
            break;
        case TSToken::Type::MATH_SYMBOL:
            printf("%-20s | %s\n", "Math Symbol", lexeme.c_str());
            break;
        case TSToken::Type::SEGMENT_DIRECTIVE:
            printf("%-20s | %s\n", "Segment Directive", lexeme.c_str());
            break;
        case TSToken::Type::INSTRUCTION:
            printf("%-20s | %s\n", "Instruction", lexeme.c_str());
            break;
        case TSToken::Type::REGISTER_8:
            printf("%-20s | %s\n", "Register 8", lexeme.c_str());
            break;
        case TSToken::Type::REGISTER_32:
            printf("%-20s | %s\n", "Register 32", lexeme.c_str());
            break;
        case TSToken::Type::REGISTER_SEGMENT:
            printf("%-20s | %s\n", "Register Segment", lexeme.c_str());
            break;
        case TSToken::Type::SIZE_IDENTIFIER:
            printf("%-20s | %s\n", "Size Identifier", lexeme.c_str());
            break;
        case TSToken::Type::DATA_IDENTIFIER:
            printf("%-20s | %s\n", "Data Identifier", lexeme.c_str());
            break;
        case TSToken::Type::CONSTANT_NUMBER:
            printf("%-20s | %s\n", "Constant Number", lexeme.c_str());
            break;
        case TSToken::Type::CONSTANT_STRING:
            printf("%-20s | %s\n", "Constant String", lexeme.c_str());
            break;
        case TSToken::Type::CONDITION_DIRECTIVE:
            printf("%-20s | %s\n", "Condition Directive", lexeme.c_str());
            break;
        case TSToken::Type::CONDITION:
            printf("%-20s | %s\n", "Condition", lexeme.c_str());
            break;
        case TSToken::Type::COMMA:
            printf("%-20s | %s\n", "Comma", lexeme.c_str());
            break;
        case TSToken::Type::COLON:
            printf("%-20s | %s\n", "Colon", lexeme.c_str());
            break;
        case TSToken::Type::SIZE_OPERATOR:
            printf("%-20s | %s\n", "Size Operator", lexeme.c_str());
            break;
        case TSToken::Type::EQU_DIRECTIVE:
            printf("%-20s | %s\n", "EQU Directive", lexeme.c_str());
            break;
        case TSToken::Type::END_DIRECTIVE:
            printf("%-20s | %s\n", "END Directive", lexeme.c_str());
            break;
        default:
            printf("%-20s | %s\n", "Undefined", "Undefined");
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
