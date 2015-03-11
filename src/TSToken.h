#ifndef _TSTOKEN_H_
#define _TSTOKEN_H_

#include "TSGlobal.h"
#include "TSLexicalAnalyze.h"

class TSToken
{
public:
    enum class Type
    {
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

    TSToken() = delete;
    TSToken(TSLexemeContainer &lexemeContainer);
    TSToken(const TSToken &token);
    TSToken &operator=(const TSToken &token);
    Type getType() const;
    string stringValue() const;
    SingleChar singleCharValue() const;
    Directive directiveValue() const;
    Instruction instructionValue() const;
    Register8 register8Value() const;
    Register32 register32Value() const;
    RegisterSegment registerSegmentValue() const;
    SizeIdentifier sizeIdentifierValue() const;
    DataIdentifier dataIdentifierValue() const;
    long long numberValue() const;
    Condition conditionValue() const;
    ~TSToken();
private:
    Type type;
    void *dataP;
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
};

struct TSTokenContainer
{
    const uint row;
    const uint column;
    const uint length;
    const TSToken token;
};

vector<TSTokenContainer> constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector);

#endif
