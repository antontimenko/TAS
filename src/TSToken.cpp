#include "TSToken.h"

#include "TSException.h"
#include <stdexcept>

const map<string, TSToken::MemoryBracket> TSToken::memoryBracketMap = {
    {"[", TSToken::MemoryBracket::OPEN},
    {"]", TSToken::MemoryBracket::CLOSE}
};

const map<string, TSToken::MathSymbol> TSToken::mathSymbolMap = {
    {"+", TSToken::MathSymbol::PLUS},
    {"-", TSToken::MathSymbol::MINUS},
    {"*", TSToken::MathSymbol::MULTIPLY},
    {"/", TSToken::MathSymbol::DIVIDE},
    {"(", TSToken::MathSymbol::BRACKET_OPEN},
    {")", TSToken::MathSymbol::BRACKET_CLOSE},
};

const map<string, TSToken::SegmentDirective> TSToken::segmentDirectiveMap = {
    {"SEGMENT", TSToken::SegmentDirective::SEGMENT},
    {"ENDS", TSToken::SegmentDirective::ENDS}
};

const map<string, TSToken::Instruction> &TSToken::instructionMap = TSInstruction::instructionMap;

const map<string, TSToken::Register8> TSToken::register8Map = {
    {"AL", TSToken::Register8::AL},
    {"AH", TSToken::Register8::AH},
    {"BL", TSToken::Register8::BL},
    {"BH", TSToken::Register8::BH},
    {"CL", TSToken::Register8::CL},
    {"CH", TSToken::Register8::CH},
    {"DL", TSToken::Register8::DL},
    {"DH", TSToken::Register8::DH}
};

const map<string, TSToken::Register16> TSToken::register16Map = {
    {"AX", TSToken::Register16::AX},
    {"BX", TSToken::Register16::BX},
    {"CX", TSToken::Register16::CX},
    {"DX", TSToken::Register16::DX},
    {"SP", TSToken::Register16::SP},
    {"BP", TSToken::Register16::BP},
    {"SI", TSToken::Register16::SI},
    {"DI", TSToken::Register16::DI}
};

const map<string, TSToken::Register32> TSToken::register32Map = {
    {"EAX", TSToken::Register32::EAX},
    {"EBX", TSToken::Register32::EBX},
    {"ECX", TSToken::Register32::ECX},
    {"EDX", TSToken::Register32::EDX},
    {"ESP", TSToken::Register32::ESP},
    {"EBP", TSToken::Register32::EBP},
    {"ESI", TSToken::Register32::ESI},
    {"EDI", TSToken::Register32::EDI}
};


const map<string, TSToken::RegisterSegment> TSToken::registerSegmentMap = {
    {"CS", TSToken::RegisterSegment::CS},
    {"DS", TSToken::RegisterSegment::DS},
    {"SS", TSToken::RegisterSegment::SS},
    {"ES", TSToken::RegisterSegment::ES},
    {"FS", TSToken::RegisterSegment::FS},
    {"GS", TSToken::RegisterSegment::GS}
};

const map<string, TSToken::SizeIdentifier> TSToken::sizeIdentifierMap = {
    {"BYTE", TSToken::SizeIdentifier::BYTE},
    {"WORD", TSToken::SizeIdentifier::WORD},
    {"DWORD", TSToken::SizeIdentifier::DWORD}
};

const map<string, TSToken::DataIdentifier> TSToken::dataIdentifierMap = {
    {"DB", TSToken::DataIdentifier::DB},
    {"DW", TSToken::DataIdentifier::DW},
    {"DD", TSToken::DataIdentifier::DD}
};

const map<string, TSToken::ConditionDirective> TSToken::conditionDirectiveMap = {
    {"IF", TSToken::ConditionDirective::IF},
    {"ELSE", TSToken::ConditionDirective::ELSE},
    {"ENDIF", TSToken::ConditionDirective::ENDIF},
};

const map<string, TSToken::Condition> TSToken::conditionMap = {
    {"EQ", TSToken::Condition::EQ},
    {"NE", TSToken::Condition::NE},
    {"LT", TSToken::Condition::LT},
    {"LE", TSToken::Condition::LE},
    {"GT", TSToken::Condition::GT},
    {"GE", TSToken::Condition::GE}
};

const string TSToken::commaStr = ",";
const string TSToken::colonStr = ":";
const string TSToken::sizeOperatorStr = "PTR";
const string TSToken::equDirectiveStr = "EQU";
const string TSToken::endDirectiveStr = "END";

vector<TSTokenContainer> TSToken::constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<TSTokenContainer> tokenContainerVector;

    for (auto it = lexemeContainerVector.begin(); it != lexemeContainerVector.end(); ++it)
    {
        const TSLexemeContainer &lexemeContainer = *it;
        const string &lexeme = lexemeContainer.lexeme;
        
        TSToken currentToken;

        if (lexeme == commaStr)
            currentToken = TSToken(Type::COMMA);
        else if (lexeme == colonStr)
            currentToken = TSToken(Type::COLON);
        else if (lexeme == sizeOperatorStr)
            currentToken = TSToken(Type::SIZE_OPERATOR);
        else if (lexeme == equDirectiveStr)
            currentToken = TSToken(Type::EQU_DIRECTIVE);
        else if (lexeme == endDirectiveStr)
            currentToken = TSToken(Type::END_DIRECTIVE);
        else if (memoryBracketMap.count(lexeme))
            currentToken = TSToken(Type::MEMORY_BRACKET, memoryBracketMap.find(lexeme)->second);
        else if (mathSymbolMap.count(lexeme))
            currentToken = TSToken(Type::MATH_SYMBOL, mathSymbolMap.find(lexeme)->second);
        else if (segmentDirectiveMap.count(lexeme))
            currentToken = TSToken(Type::SEGMENT_DIRECTIVE, segmentDirectiveMap.find(lexeme)->second);
        else if (instructionMap.count(lexeme))
            currentToken = TSToken(Type::INSTRUCTION, instructionMap.find(lexeme)->second);
        else if (register8Map.count(lexeme))
            currentToken = TSToken(Type::REGISTER_8, register8Map.find(lexeme)->second);
        else if (register16Map.count(lexeme))
            currentToken = TSToken(Type::REGISTER_16, register16Map.find(lexeme)->second);
        else if (register32Map.count(lexeme))
            currentToken = TSToken(Type::REGISTER_32, register32Map.find(lexeme)->second);
        else if (registerSegmentMap.count(lexeme))
            currentToken = TSToken(Type::REGISTER_SEGMENT, registerSegmentMap.find(lexeme)->second);
        else if (sizeIdentifierMap.count(lexeme))
            currentToken = TSToken(Type::SIZE_IDENTIFIER, sizeIdentifierMap.find(lexeme)->second);
        else if (dataIdentifierMap.count(lexeme))
            currentToken = TSToken(Type::DATA_IDENTIFIER, dataIdentifierMap.find(lexeme)->second);
        else if (conditionDirectiveMap.count(lexeme))
            currentToken = TSToken(Type::CONDITION_DIRECTIVE, conditionDirectiveMap.find(lexeme)->second);
        else if (conditionMap.count(lexeme))
            currentToken = TSToken(Type::CONDITION, conditionMap.find(lexeme)->second);
        else if (isCharQuoteCompatible(lexeme[0]))
            currentToken = TSToken(Type::CONSTANT_STRING, lexeme.substr(1, lexeme.size() - 2));
        else if (isCharNumberCompatible(lexeme[0]))
        {
            longlong number;

            try
            {
                size_t stoiStop;
                size_t realNumberSize;

                if (isCharNumberCompatible(lexeme[lexeme.size() - 1]))
                {
                    number = std::stoll(lexeme, &stoiStop, 10);
                    realNumberSize = lexeme.size();
                }
                else if (lexeme[lexeme.size() - 1] == 'D')
                {
                    number = std::stoll(lexeme, &stoiStop, 10);
                    realNumberSize = lexeme.size() - 1;
                }
                else if (lexeme[lexeme.size() - 1] == 'B')
                {
                    number = std::stoll(lexeme, &stoiStop, 2);
                    realNumberSize = lexeme.size() - 1;
                }
                else if (lexeme[lexeme.size() - 1] == 'H')
                {
                    number = std::stoll(lexeme, &stoiStop, 16);
                    realNumberSize = lexeme.size() - 1;
                }
                else
                    throw TSCompileError("Invalid numeric constant", lexemeContainer.row, lexemeContainer.column, lexeme.size());
                
                if (stoiStop != realNumberSize)
                    throw TSCompileError("Invalid numeric constant", lexemeContainer.row, lexemeContainer.column, lexeme.size());
            }
            catch (std::exception &e)
            {
                throw TSCompileError("Invalid numeric constant", lexemeContainer.row, lexemeContainer.column, lexeme.size());
            }

            currentToken = TSToken(Type::CONSTANT_NUMBER, number);
        }
        else
            currentToken = TSToken(Type::USER_IDENTIFIER, lexeme);

        tokenContainerVector.push_back({it->row,
                                        it->column,
                                        it->lexeme.size(),
                                        currentToken});
    }

    return tokenContainerVector;
}
