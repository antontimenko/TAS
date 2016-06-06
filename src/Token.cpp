#include "Token.h"

#include "Exception.h"
#include <stdexcept>

const map<string, Token::MemoryBracket> Token::memoryBracketMap = {
    {"[", Token::MemoryBracket::OPEN},
    {"]", Token::MemoryBracket::CLOSE}
};

const map<string, Token::MathSymbol> Token::mathSymbolMap = {
    {"+", Token::MathSymbol::PLUS},
    {"-", Token::MathSymbol::MINUS},
    {"*", Token::MathSymbol::MULTIPLY},
    {"/", Token::MathSymbol::DIVIDE},
    {"(", Token::MathSymbol::BRACKET_OPEN},
    {")", Token::MathSymbol::BRACKET_CLOSE},
};

const map<string, Token::Instruction> &Token::instructionMap = InstructionNS::instructionMap;

const map<string, Token::Register> &Token::registerMap = OperandMask::registerMap;

const map<string, Token::SizeIdentifier> Token::sizeIdentifierMap = {
    {"BYTE", Token::SizeIdentifier::BYTE},
    {"WORD", Token::SizeIdentifier::WORD},
    {"DWORD", Token::SizeIdentifier::DWORD}
};

const map<string, Token::DataIdentifier> &Token::dataIdentifierMap = InstructionNS::dataIdentifierMap;

const map<string, Token::ConditionDirective> Token::conditionDirectiveMap = {
    {"IF", Token::ConditionDirective::IF},
    {"ELSE", Token::ConditionDirective::ELSE},
    {"ENDIF", Token::ConditionDirective::ENDIF},
};

const map<string, Token::Condition> Token::conditionMap = {
    {"EQ", Token::Condition::EQ},
    {"NE", Token::Condition::NE},
    {"LT", Token::Condition::LT},
    {"LE", Token::Condition::LE},
    {"GT", Token::Condition::GT},
    {"GE", Token::Condition::GE}
};

const string Token::segmentStr = "SEGMENT";
const string Token::endsStr = "ENDS";
const string Token::commaStr = ",";
const string Token::colonStr = ":";
const string Token::sizeOperatorStr = "PTR";
const string Token::equDirectiveStr = "EQU";
const string Token::endDirectiveStr = "END";
const string Token::assumeDirectiveStr = "ASSUME";

vector<TokenContainer> constructTokenContainerVector(vector<LexemeContainer> &lexemeContainerVector) {
    vector<TokenContainer> tokenContainerVector;

    for (auto it = lexemeContainerVector.begin(); it != lexemeContainerVector.end(); ++it) {
        const LexemeContainer &lexemeContainer = *it;
        const string &lexeme = lexemeContainer.lexeme;
        
        Token currentToken;

        if (lexeme == Token::segmentStr)
            currentToken = Token(Token::Type::SEGMENT_DIRECTIVE);
        else if (lexeme == Token::endsStr)
            currentToken = Token(Token::Type::ENDS_DIRECTIVE);
        else if (lexeme == Token::commaStr)
            currentToken = Token(Token::Type::COMMA);
        else if (lexeme == Token::colonStr)
            currentToken = Token(Token::Type::COLON);
        else if (lexeme == Token::sizeOperatorStr)
            currentToken = Token(Token::Type::SIZE_OPERATOR);
        else if (lexeme == Token::equDirectiveStr)
            currentToken = Token(Token::Type::EQU_DIRECTIVE);
        else if (lexeme == Token::endDirectiveStr)
            currentToken = Token(Token::Type::END_DIRECTIVE);
        else if (lexeme == Token::assumeDirectiveStr)
            currentToken = Token(Token::Type::ASSUME_DIRECTIVE);
        else if (Token::memoryBracketMap.count(lexeme))
            currentToken = Token(Token::Type::MEMORY_BRACKET, Token::memoryBracketMap.find(lexeme)->second);
        else if (Token::mathSymbolMap.count(lexeme))
            currentToken = Token(Token::Type::MATH_SYMBOL, Token::mathSymbolMap.find(lexeme)->second);
        else if (Token::instructionMap.count(lexeme))
            currentToken = Token(Token::Type::INSTRUCTION, Token::instructionMap.find(lexeme)->second);
        else if (Token::registerMap.count(lexeme))
            currentToken = Token(Token::Type::REGISTER, Token::registerMap.find(lexeme)->second);
        else if (Token::sizeIdentifierMap.count(lexeme))
            currentToken = Token(Token::Type::SIZE_IDENTIFIER, Token::sizeIdentifierMap.find(lexeme)->second);
        else if (Token::dataIdentifierMap.count(lexeme))
            currentToken = Token(Token::Type::DATA_IDENTIFIER, Token::dataIdentifierMap.find(lexeme)->second);
        else if (Token::conditionDirectiveMap.count(lexeme))
            currentToken = Token(Token::Type::CONDITION_DIRECTIVE, Token::conditionDirectiveMap.find(lexeme)->second);
        else if (Token::conditionMap.count(lexeme))
            currentToken = Token(Token::Type::CONDITION, Token::conditionMap.find(lexeme)->second);
        else if (isCharQuoteCompatible(lexeme[0]))
            currentToken = Token(Token::Type::CONSTANT_STRING, lexeme.substr(1, lexeme.size() - 2));
        else if (isCharNumberCompatible(lexeme[0])) {
            Integer number;

            try {
                size_t stoiStop;
                size_t realNumberSize;

                if (isCharNumberCompatible(lexeme[lexeme.size() - 1])) {
                    number = std::stoull(lexeme, &stoiStop, 10);
                    realNumberSize = lexeme.size();
                } else if (lexeme[lexeme.size() - 1] == 'D') {
                    number = std::stoull(lexeme, &stoiStop, 10);
                    realNumberSize = lexeme.size() - 1;
                } else if (lexeme[lexeme.size() - 1] == 'B') {
                    number = std::stoull(lexeme, &stoiStop, 2);
                    realNumberSize = lexeme.size() - 1;
                } else if (lexeme[lexeme.size() - 1] == 'H') {
                    number = std::stoull(lexeme, &stoiStop, 16);
                    realNumberSize = lexeme.size() - 1;
                } else
                    throw CompileError("Invalid numeric constant", {lexemeContainer.row,
                                                                      lexemeContainer.column,
                                                                      lexeme.size()});
                
                if (stoiStop != realNumberSize)
                    throw CompileError("Invalid numeric constant", {lexemeContainer.row,
                                                                      lexemeContainer.column,
                                                                      lexeme.size()});
            } catch (std::exception &e) {
                throw CompileError("Invalid numeric constant", {lexemeContainer.row,
                                                                  lexemeContainer.column,
                                                                  lexeme.size()});
            }

            currentToken = Token(Token::Type::CONSTANT_NUMBER, number);
        } else
            currentToken = Token(Token::Type::USER_IDENTIFIER, lexeme);

        tokenContainerVector.push_back({{it->row,
                                         it->column,
                                         it->lexeme.size()}, currentToken});
    }

    return tokenContainerVector;
}
