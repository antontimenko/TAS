#ifndef _TSTOKEN_H_
#define _TSTOKEN_H_

#include "TSGlobal.h"
#include "TSLexeme.h"

struct TSTokenContainer;

class TSToken
{
public:
    enum class Type
    {
        null,
        userIdentifier,
        singleChar,
        directive,
        instruction,
        register8,
        register32,
        registerSegment,
        sizeOperator,
        sizeIdentifier,
        dataIdentifier,
        constantNumber,
        constantString,
        condition
    };

    enum class SingleChar
    {
        COMMA,
        COLON,
        SQUARE_BRACKET_OPEN,
        SQUARE_BRACKET_CLOSE,
        ROUND_BRACKET_OPEN,
        ROUND_BRACKET_CLOSE,
        PLUS,
        MINUS
    };

    enum class Directive
    {
        SEGMENT,
        ENDS,
        EQU,
        IF,
        ELSE,
        ENDIF,
        END
    };

    enum class Register8
    {
        AL,
        AH,
        BL,
        BH,
        CL,
        CH,
        DL,
        DH
    };

    enum class Register32
    {
        EAX,
        EBX,
        ECX,
        EDX,
        ESP,
        EBP,
        ESI,
        EDI
    };

    enum class RegisterSegment
    {
        CS,
        DS,
        SS,
        ES,
        FS,
        GS
    };

    enum class SizeIdentifier
    {
        BYTE,
        DWORD
    };

    enum class DataIdentifier
    {
        DB,
        DW,
        DD
    };

    enum class Condition
    {
        EQ,
        NE,
        LT,
        LE,
        GT,
        GE
    };

    enum class Instruction
    {
        SCASD,
        RCL,
        DIV,
        OR,
        CMP,
        AND,
        MOV,
        ADD,
        JNB
    };

    template<typename T>
    inline TSToken(Type tokenType, T value) :
        _type(tokenType),
        valueP(new T(value))
    {
    }
    inline TSToken(Type tokenType) :
        TSToken(tokenType, nullptr)
    {
    }
    inline TSToken() :
        TSToken(Type::null)
    {
    }
    inline Type type() const
    {
        return _type;
    }
    template<typename T>
    inline T value() const
    {
        return static_cast<T>(*valueP);
    }
    static const map<string, SingleChar> singleCharMap;
    static const map<string, Directive> directiveMap;
    static const map<string, Register8> register8Map;
    static const map<string, Register32> register32Map;
    static const map<string, RegisterSegment> registerSegmentMap;
    static const map<string, SizeIdentifier> sizeIdentifierMap;
    static const map<string, DataIdentifier> dataIdentifierMap;
    static const map<string, Condition> conditionMap;
    static const map<string, Instruction> instructionMap;
    static const string sizeOperatorStr;
    static vector<TSTokenContainer> constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector);
private:
    Type _type;
    shared_ptr<void> valueP;
};

struct TSTokenContainer
{
    const size_t row;
    const size_t column;
    const size_t length;
    const TSToken token;
};

#endif
