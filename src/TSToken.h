#ifndef _TSTOKEN_H_
#define _TSTOKEN_H_

#include "TSGlobal.h"
#include "TSLexeme.h"
#include "TSUniquePtr.h"
#include <utility>

class TSTokenContainer;

class TSToken
{
public:
    enum class Type
    {
        null,
        userIdentifier,
        singleChar,
        mathSymbol,
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
        BRACKET_OPEN,
        BRACKET_CLOSE
    };

    enum class MathSymbol
    {
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
        BRACKET_OPEN,
        BRACKET_CLOSE
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
    inline TSToken(const TSToken &token) :
        _type(token._type),
        valueP(token.valueP.copy())
    {
    }
    inline TSToken &operator=(const TSToken &token)
    {
        _type = token._type;
        valueP = token.valueP.copy();

        return *this;
    }
    inline bool operator==(const TSToken &token)
    {
        return (_type == token._type) && valueP.compareContents(token.valueP);
    }
    inline Type type() const
    {
        return _type;
    }
    template<typename T>
    inline T value() const
    {
        return *static_cast<T *>(valueP.get());
    }
    static const map<string, SingleChar> singleCharMap;
    static const map<string, MathSymbol> mathSymbolMap;
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
    TSUniquePtr<void> valueP;
};

class TSTokenContainer
{
public:
    size_t row;
    size_t column;
    size_t length;
    TSToken token;
    inline bool operator==(const TSTokenContainer &tokenContainer)
    {
        return (row == tokenContainer.row) &&
               (column == tokenContainer.column) &&
               (length == tokenContainer.length) &&
               (token == tokenContainer.token);
    }
};

#endif
