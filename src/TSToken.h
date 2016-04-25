#ifndef _TSTOKEN_H_
#define _TSTOKEN_H_

#include "TSGlobal.h"
#include "TSCodePosition.h"
#include "TSLexeme.h"
#include "TSUniquePtr.h"
#include "TSInstruction.h"

class TSToken {
public:
    enum class Type {
        USER_IDENTIFIER,
        MEMORY_BRACKET,
        MATH_SYMBOL,
        COMMA,
        COLON,
        SEGMENT_DIRECTIVE,
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

    enum class SegmentDirective {
        SEGMENT,
        ENDS
    };

    typedef TSInstruction::Instruction Instruction;

    typedef TSOperandMask::Mask Register;

    enum class SizeIdentifier {
        BYTE,
        WORD,
        DWORD
    };

    typedef TSInstruction::DataIdentifier DataIdentifier;

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
    inline TSToken(Type tokenType, T value) :
        _type(tokenType),
        valueP(new T(value))
    {}

    inline TSToken(Type tokenType) :
        TSToken(tokenType, nullptr)
    {}

    inline TSToken() :
        TSToken(Type::USER_IDENTIFIER)
    {}

    inline TSToken(const TSToken &token) :
        _type(token._type),
        valueP(token.valueP.copy())
    {}

    inline TSToken &operator=(const TSToken &token) {
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
    static const map<string, SegmentDirective> segmentDirectiveMap;
    static const map<string, Instruction> &instructionMap;
    static const map<string, Register> &registerMap;
    static const map<string, SizeIdentifier> sizeIdentifierMap;
    static const map<string, DataIdentifier> &dataIdentifierMap;
    static const map<string, ConditionDirective> conditionDirectiveMap;
    static const map<string, Condition> conditionMap;
    static const string commaStr;
    static const string colonStr;
    static const string sizeOperatorStr;
    static const string equDirectiveStr;
    static const string endDirectiveStr;
    static const string assumeDirectiveStr;
private:
    Type _type;
    TSUniquePtr<void> valueP;
};

class TSTokenContainer {
public:
    TSCodePosition pos;
    TSToken token;
    inline bool operator==(const TSTokenContainer &tokenContainer) const {
        return pos == tokenContainer.pos;
    }
};

vector<TSTokenContainer> constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector);

#endif
