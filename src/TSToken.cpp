#include "TSToken.h"

#include "TSException.h"
#include <stdexcept>

const map<string, TSToken::SingleChar> TSToken::singleCharMap = {
    {",", TSToken::SingleChar::COMMA},
    {":", TSToken::SingleChar::COLON},
    {"[", TSToken::SingleChar::BRACKET_OPEN},
    {"]", TSToken::SingleChar::BRACKET_CLOSE}
};

const map<string, TSToken::MathSymbol> TSToken::mathSymbolMap = {
    {"+", TSToken::MathSymbol::PLUS},
    {"-", TSToken::MathSymbol::MINUS},
    {"*", TSToken::MathSymbol::MULTIPLY},
    {"/", TSToken::MathSymbol::DIVIDE},
    {"(", TSToken::MathSymbol::BRACKET_OPEN},
    {")", TSToken::MathSymbol::BRACKET_CLOSE},
};

const map<string, TSToken::Directive> TSToken::directiveMap = {
    {"SEGMENT", TSToken::Directive::SEGMENT},
    {"ENDS", TSToken::Directive::ENDS},
    {"EQU", TSToken::Directive::EQU},
    {"IF", TSToken::Directive::IF},
    {"ELSE", TSToken::Directive::ELSE},
    {"ENDIF", TSToken::Directive::ENDIF},
    {"END", TSToken::Directive::END}
};

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
    {"DWORD", TSToken::SizeIdentifier::DWORD}
};

const map<string, TSToken::DataIdentifier> TSToken::dataIdentifierMap = {
    {"DB", TSToken::DataIdentifier::DB},
    {"DW", TSToken::DataIdentifier::DW},
    {"DD", TSToken::DataIdentifier::DD}
};

const map<string, TSToken::Condition> TSToken::conditionMap = {
    {"EQ", TSToken::Condition::EQ},
    {"NE", TSToken::Condition::NE},
    {"LT", TSToken::Condition::LT},
    {"LE", TSToken::Condition::LE},
    {"GT", TSToken::Condition::GT},
    {"GE", TSToken::Condition::GE}
};

const map<string, TSToken::Instruction> TSToken::instructionMap = {
    {"SCASD", TSToken::Instruction::SCASD},
    {"RCL", TSToken::Instruction::RCL},
    {"DIV", TSToken::Instruction::DIV},
    {"OR", TSToken::Instruction::OR},
    {"CMP", TSToken::Instruction::CMP},
    {"AND", TSToken::Instruction::AND},
    {"MOV", TSToken::Instruction::MOV},
    {"ADD", TSToken::Instruction::ADD},
    {"JNB", TSToken::Instruction::JNB}
};

const string TSToken::sizeOperatorStr = "PTR";

vector<TSTokenContainer> TSToken::constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<TSTokenContainer> tokenContainerVector;

    for (auto it = lexemeContainerVector.begin(); it != lexemeContainerVector.end(); ++it)
    {
        const TSLexemeContainer &lexemeContainer = *it;
        const string &lexeme = lexemeContainer.lexeme;
        
        TSToken currentToken;

        if (lexeme == sizeOperatorStr)
            currentToken = TSToken(Type::sizeOperator);
        else if (singleCharMap.count(lexeme))
            currentToken = TSToken(Type::singleChar, singleCharMap.find(lexeme)->second);
        else if (mathSymbolMap.count(lexeme))
            currentToken = TSToken(Type::mathSymbol, mathSymbolMap.find(lexeme)->second);
        else if (directiveMap.count(lexeme))
            currentToken = TSToken(Type::directive, directiveMap.find(lexeme)->second);
        else if (register8Map.count(lexeme))
            currentToken = TSToken(Type::register8, register8Map.find(lexeme)->second);
        else if (register32Map.count(lexeme))
            currentToken = TSToken(Type::register32, register32Map.find(lexeme)->second);
        else if (registerSegmentMap.count(lexeme))
            currentToken = TSToken(Type::registerSegment, registerSegmentMap.find(lexeme)->second);
        else if (sizeIdentifierMap.count(lexeme))
            currentToken = TSToken(Type::sizeIdentifier, sizeIdentifierMap.find(lexeme)->second);
        else if (dataIdentifierMap.count(lexeme))
            currentToken = TSToken(Type::dataIdentifier, dataIdentifierMap.find(lexeme)->second);
        else if (conditionMap.count(lexeme))
            currentToken = TSToken(Type::condition, conditionMap.find(lexeme)->second);
        else if (instructionMap.count(lexeme))
            currentToken = TSToken(Type::instruction, instructionMap.find(lexeme)->second);
        else if (isCharQuoteCompatible(lexeme[0]))
            currentToken = TSToken(Type::constantString, lexeme.substr(1, lexeme.size() - 2));
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

            currentToken = TSToken(Type::constantNumber, number);
        }
        else
            currentToken = TSToken(Type::userIdentifier, lexeme);

        tokenContainerVector.push_back({it->row,
                                        it->column,
                                        it->lexeme.size(),
                                        currentToken});
    }

    return tokenContainerVector;
}
