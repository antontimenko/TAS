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

const map<string, TSToken::Register> &TSToken::registerMap = TSOperandMask::registerMap;

const map<string, TSToken::SizeIdentifier> TSToken::sizeIdentifierMap = {
    {"BYTE", TSToken::SizeIdentifier::BYTE},
    {"WORD", TSToken::SizeIdentifier::WORD},
    {"DWORD", TSToken::SizeIdentifier::DWORD}
};

const map<string, TSToken::DataIdentifier> &TSToken::dataIdentifierMap = TSInstruction::dataIdentifierMap;

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

vector<TSTokenContainer> constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<TSTokenContainer> tokenContainerVector;

    for (auto it = lexemeContainerVector.begin(); it != lexemeContainerVector.end(); ++it)
    {
        const TSLexemeContainer &lexemeContainer = *it;
        const string &lexeme = lexemeContainer.lexeme;
        
        TSToken currentToken;

        if (lexeme == TSToken::commaStr)
            currentToken = TSToken(TSToken::Type::COMMA);
        else if (lexeme == TSToken::colonStr)
            currentToken = TSToken(TSToken::Type::COLON);
        else if (lexeme == TSToken::sizeOperatorStr)
            currentToken = TSToken(TSToken::Type::SIZE_OPERATOR);
        else if (lexeme == TSToken::equDirectiveStr)
            currentToken = TSToken(TSToken::Type::EQU_DIRECTIVE);
        else if (lexeme == TSToken::endDirectiveStr)
            currentToken = TSToken(TSToken::Type::END_DIRECTIVE);
        else if (TSToken::memoryBracketMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::MEMORY_BRACKET, TSToken::memoryBracketMap.find(lexeme)->second);
        else if (TSToken::mathSymbolMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::MATH_SYMBOL, TSToken::mathSymbolMap.find(lexeme)->second);
        else if (TSToken::segmentDirectiveMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::SEGMENT_DIRECTIVE, TSToken::segmentDirectiveMap.find(lexeme)->second);
        else if (TSToken::instructionMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::INSTRUCTION, TSToken::instructionMap.find(lexeme)->second);
        else if (TSToken::registerMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::REGISTER, TSToken::registerMap.find(lexeme)->second);
        else if (TSToken::sizeIdentifierMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::SIZE_IDENTIFIER, TSToken::sizeIdentifierMap.find(lexeme)->second);
        else if (TSToken::dataIdentifierMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::DATA_IDENTIFIER, TSToken::dataIdentifierMap.find(lexeme)->second);
        else if (TSToken::conditionDirectiveMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::CONDITION_DIRECTIVE, TSToken::conditionDirectiveMap.find(lexeme)->second);
        else if (TSToken::conditionMap.count(lexeme))
            currentToken = TSToken(TSToken::Type::CONDITION, TSToken::conditionMap.find(lexeme)->second);
        else if (isCharQuoteCompatible(lexeme[0]))
            currentToken = TSToken(TSToken::Type::CONSTANT_STRING, lexeme.substr(1, lexeme.size() - 2));
        else if (isCharNumberCompatible(lexeme[0]))
        {
            TSInteger number;

            try
            {
                size_t stoiStop;
                size_t realNumberSize;

                if (isCharNumberCompatible(lexeme[lexeme.size() - 1]))
                {
                    number = std::stoull(lexeme, &stoiStop, 10);
                    realNumberSize = lexeme.size();
                }
                else if (lexeme[lexeme.size() - 1] == 'D')
                {
                    number = std::stoull(lexeme, &stoiStop, 10);
                    realNumberSize = lexeme.size() - 1;
                }
                else if (lexeme[lexeme.size() - 1] == 'B')
                {
                    number = std::stoull(lexeme, &stoiStop, 2);
                    realNumberSize = lexeme.size() - 1;
                }
                else if (lexeme[lexeme.size() - 1] == 'H')
                {
                    number = std::stoull(lexeme, &stoiStop, 16);
                    realNumberSize = lexeme.size() - 1;
                }
                else
                    throw TSCompileError("Invalid numeric constant", {lexemeContainer.row,
                                                                      lexemeContainer.column,
                                                                      lexeme.size()});
                
                if (stoiStop != realNumberSize)
                    throw TSCompileError("Invalid numeric constant", {lexemeContainer.row,
                                                                      lexemeContainer.column,
                                                                      lexeme.size()});
            }
            catch (std::exception &e)
            {
                throw TSCompileError("Invalid numeric constant", {lexemeContainer.row,
                                                                  lexemeContainer.column,
                                                                  lexeme.size()});
            }

            currentToken = TSToken(TSToken::Type::CONSTANT_NUMBER, number);
        }
        else
            currentToken = TSToken(TSToken::Type::USER_IDENTIFIER, lexeme);

        tokenContainerVector.push_back({{it->row,
                                         it->column,
                                         it->lexeme.size()}, currentToken});
    }

    return tokenContainerVector;
}
