#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "Global.h"
#include "CodePosition.h"
#include "Lexeme.h"
#include "UniquePtr.h"
#include "Instruction.h"

class Token {
public:
    enum class Type {
        USER_IDENTIFIER,
        MEMORY_BRACKET,
        MATH_SYMBOL,
        COMMA,
        COLON,
        SEGMENT_DIRECTIVE,
        ENDS_DIRECTIVE,
        INSTRUCTION,
        REGISTER,
        SIZE_IDENTIFIER,
        DATA_IDENTIFIER,
        CONSTANT_NUMBER,
        CONSTANT_STRING,
        CONDITION_DIRECTIVE,
        CONDITION,
        SIZE_OPERATOR,
        EQU_DIRECTIVE,
        END_DIRECTIVE,
        ASSUME_DIRECTIVE
    };

    enum class MemoryBracket {
        OPEN,
        CLOSE
    };

    enum class MathSymbol {
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        BRACKET_OPEN,
        BRACKET_CLOSE
    };

    typedef InstructionNS::Instruction Instruction;

    typedef OperandMask::Mask Register;

    enum class SizeIdentifier {
        BYTE,
        WORD,
        DWORD
    };

    typedef InstructionNS::DataIdentifier DataIdentifier;

    enum class ConditionDirective {
        IF,
        ELSE,
        ENDIF
    };

    enum class Condition {
        EQ,
        NE,
        LT,
        LE,
        GT,
        GE
    };

    template<typename T>
    inline Token(Type tokenType, T value) :
        _type(tokenType),
        valueP(new T(value))
    {}

    inline Token(Type tokenType) :
        Token(tokenType, nullptr)
    {}

    inline Token() :
        Token(Type::USER_IDENTIFIER)
    {}

    inline Token(const Token &token) :
        _type(token._type),
        valueP(token.valueP.copy())
    {}

    inline Token &operator=(const Token &token) {
        _type = token._type;
        valueP = token.valueP.copy();

        return *this;
    }

    inline Type type() const {
        return _type;
    }

    template<typename T>
    inline T value() const {
        return *static_cast<T *>(valueP.get());
    }
    
    static const map<string, MemoryBracket> memoryBracketMap;
    static const map<string, MathSymbol> mathSymbolMap;
    static const map<string, Instruction> &instructionMap;
    static const map<string, Register> &registerMap;
    static const map<string, SizeIdentifier> sizeIdentifierMap;
    static const map<string, DataIdentifier> &dataIdentifierMap;
    static const map<string, ConditionDirective> conditionDirectiveMap;
    static const map<string, Condition> conditionMap;
    static const string segmentStr;
    static const string endsStr;
    static const string commaStr;
    static const string colonStr;
    static const string sizeOperatorStr;
    static const string equDirectiveStr;
    static const string endDirectiveStr;
    static const string assumeDirectiveStr;
private:
    Type _type;
    UniquePtr<void> valueP;
};

class TokenContainer {
public:
    CodePosition pos;
    Token token;
    inline bool operator==(const TokenContainer &tokenContainer) const {
        return pos == tokenContainer.pos;
    }
};

vector<TokenContainer> constructTokenContainerVector(vector<LexemeContainer> &lexemeContainerVector);

#endif
