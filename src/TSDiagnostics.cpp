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

const map<TSToken::Type, string> tokenTypeDescriptionMap = {
    {TSToken::Type::UNDEFINED, "Undefined"},
    {TSToken::Type::USER_IDENTIFIER, "User Identifier"},
    {TSToken::Type::MEMORY_BRACKET, "Memory Bracket"},
    {TSToken::Type::MATH_SYMBOL, "Math Symbol"},
    {TSToken::Type::COMMA, "Comma"},
    {TSToken::Type::COLON, "Colon"},
    {TSToken::Type::SEGMENT_DIRECTIVE, "Segment Directive"},
    {TSToken::Type::INSTRUCTION, "Instruction"},
    {TSToken::Type::REGISTER_8, "Register 8"},
    {TSToken::Type::REGISTER_16, "Register 16"},
    {TSToken::Type::REGISTER_32, "Register 32"},
    {TSToken::Type::REGISTER_SEGMENT, "Register Segment"},
    {TSToken::Type::SIZE_IDENTIFIER, "Size Identifier"},
    {TSToken::Type::DATA_IDENTIFIER, "Data Identifier"},
    {TSToken::Type::CONSTANT_NUMBER, "Constant Number"},
    {TSToken::Type::CONSTANT_STRING, "Constant String"},
    {TSToken::Type::CONDITION_DIRECTIVE, "Condition Directive"},
    {TSToken::Type::CONDITION, "Condition"},
    {TSToken::Type::SIZE_OPERATOR, "Size Operator"},
    {TSToken::Type::EQU_DIRECTIVE, "EQU Directive"},
    {TSToken::Type::END_DIRECTIVE, "END Directive"}
};

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

string getTokenString(const TSToken &token)
{
    string returnString;

    switch (token.type())
    {
    case TSToken::Type::USER_IDENTIFIER:
        returnString = token.value<string>();
        break;
    case TSToken::Type::MEMORY_BRACKET:
        returnString = findByValue(TSToken::memoryBracketMap, token.value<TSToken::MemoryBracket>())->first;
        break;
    case TSToken::Type::MATH_SYMBOL:
        returnString = findByValue(TSToken::mathSymbolMap, token.value<TSToken::MathSymbol>())->first;
        break;
    case TSToken::Type::SEGMENT_DIRECTIVE:
        returnString = findByValue(TSToken::segmentDirectiveMap, token.value<TSToken::SegmentDirective>())->first;
        break;
    case TSToken::Type::INSTRUCTION:
        returnString = findByValue(TSToken::instructionMap, token.value<TSToken::Instruction>())->first;
        break;
    case TSToken::Type::REGISTER_8:
        returnString = findByValue(TSToken::register8Map, token.value<TSToken::Register8>())->first;
        break;
    case TSToken::Type::REGISTER_16:
        returnString = findByValue(TSToken::register16Map, token.value<TSToken::Register16>())->first;
        break;
    case TSToken::Type::REGISTER_32:
        returnString = findByValue(TSToken::register32Map, token.value<TSToken::Register32>())->first;
        break;
    case TSToken::Type::REGISTER_SEGMENT:
        returnString = findByValue(TSToken::registerSegmentMap, token.value<TSToken::RegisterSegment>())->first;
        break;
    case TSToken::Type::SIZE_IDENTIFIER:
        returnString = findByValue(TSToken::sizeIdentifierMap, token.value<TSToken::SizeIdentifier>())->first;
        break;
    case TSToken::Type::DATA_IDENTIFIER:
        returnString = findByValue(TSToken::dataIdentifierMap, token.value<TSToken::DataIdentifier>())->first;
        break;
    case TSToken::Type::CONSTANT_NUMBER:
        returnString = std::to_string(token.value<longlong>());
        break;
    case TSToken::Type::CONSTANT_STRING:
        returnString = token.value<string>();
        break;
    case TSToken::Type::CONDITION_DIRECTIVE:
        returnString = findByValue(TSToken::conditionDirectiveMap, token.value<TSToken::ConditionDirective>())->first;
        break;
    case TSToken::Type::CONDITION:
        returnString = findByValue(TSToken::conditionMap, token.value<TSToken::Condition>())->first;
        break;
    case TSToken::Type::COMMA:
        returnString = TSToken::commaStr;
        break;
    case TSToken::Type::COLON:
        returnString = TSToken::colonStr;
        break;
    case TSToken::Type::SIZE_OPERATOR:
        returnString = TSToken::sizeOperatorStr;
        break;
    case TSToken::Type::EQU_DIRECTIVE:
        returnString = TSToken::equDirectiveStr;
        break;
    case TSToken::Type::END_DIRECTIVE:
        returnString = TSToken::endDirectiveStr;
        break;
    default:
        returnString = "Undefined";
        break;
    }

    return returnString;
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
        printf("%-4u | %-*s | %-20s | %s\n", i,
                                             maxCoordsSize,
                                             coordsStringVector[i].c_str(),
                                             tokenTypeDescriptionMap.find(tokenContainerVector[i].token.type())->second.c_str(),
                                             getTokenString(tokenContainerVector[i].token).c_str());
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
        printf("%-4u | %-*s | %-20s | %s\n", i,
                                             maxCoordsSize,
                                             coordsStringVector[i].c_str(),
                                             tokenTypeDescriptionMap.find(tokenContainerVector[i].token.type())->second.c_str(),
                                             lexemeContainerVector[i].lexeme.c_str());
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

void printPseudoLabelTable(const map<string, tuple<TSLabelType, TSToken::DataIdentifier, size_t>> &labelMap)
{
    cout << TSColor::BWhite << "---PSEUDO LABEL TABLE OUTPUT---" << TSColor::Reset << endl << endl;

    size_t maxLabelSize = 0;
    for (auto it = labelMap.begin(); it != labelMap.end(); ++it)
    {
        if (it->first.size() > maxLabelSize)
            maxLabelSize = it->first.size();
    }

    for (auto it = labelMap.begin(); it != labelMap.end(); ++it)
        printf("%-*s | %-5s | %u\n", maxLabelSize,
                                     it->first.c_str(),
                                     std::get<0>(it->second) == TSLabelType::LABEL ? "Label" : findByValue(TSToken::dataIdentifierMap, std::get<1>(it->second))->first.c_str(),
                                     std::get<2>(it->second));

    cout << endl;
}

void printPseudoSentenceTable(const vector<TSSegmentPseudoSentence> &segmentPseudoSentenceVector)
{
    cout << TSColor::BWhite << "---PSEUDO SENTENCE TABLE OUTPUT---" << TSColor::Reset << endl << endl;

    for (auto segIt = segmentPseudoSentenceVector.begin(); segIt != segmentPseudoSentenceVector.end(); ++segIt)
    {
        cout << TSColor::BWhite << "Segment " << segIt->name << TSColor::Reset << endl << endl;

        for (auto it = segIt->pseudoSentenceVector.begin(); it != segIt->pseudoSentenceVector.end(); ++it)
        {
            printf("%-5u | %-10s || ", it - segIt->pseudoSentenceVector.begin(), 
                                       getTokenString(it->baseTokenContainer.token).c_str());

            for (auto jt = it->operandVector.begin(); jt != it->operandVector.end(); ++jt)
            {
                for (auto kt = jt->begin(); kt != jt->end(); ++kt)
                    printf("%s ", getTokenString(kt->token).c_str());

                printf("| ");
            }

            printf("\n");
        }

        printf("\n");
    }
}
