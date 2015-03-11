#include "TSToken.h"

#include "TSUtility.h"
#include "TSException.h"
#include <stdexcept>

const map<string, TSToken::SingleChar> TSToken::singleCharMap = {
    {",", TSToken::SingleChar::COMMA},
    {":", TSToken::SingleChar::COLON},
    {"[", TSToken::SingleChar::SQUARE_BRACKET_OPEN},
    {"]", TSToken::SingleChar::SQUARE_BRACKET_CLOSE},
    {"(", TSToken::SingleChar::ROUND_BRACKET_OPEN},
    {")", TSToken::SingleChar::ROUND_BRACKET_CLOSE},
    {"+", TSToken::SingleChar::PLUS},
    {"-", TSToken::SingleChar::MINUS}
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

TSToken::TSToken(TSLexemeContainer &lexemeContainer)
{
    const string &lexeme = lexemeContainer.lexeme;

    if (lexeme.compare(sizeOperatorStr) == 0)
    {
        type = Type::sizeOperator;
        dataP = nullptr;
    }
    else if (singleCharMap.find(lexeme) != singleCharMap.end())
    {
        type = Type::singleChar;
        dataP = new SingleChar(singleCharMap.find(lexeme)->second);
    }
    else if (directiveMap.find(lexeme) != directiveMap.end())
    {
        type = Type::directive;
        dataP = new Directive(directiveMap.find(lexeme)->second);
    }
    else if (register8Map.find(lexeme) != register8Map.end())
    {
        type = Type::register8;
        dataP = new Register8(register8Map.find(lexeme)->second);
    }
    else if (register32Map.find(lexeme) != register32Map.end())
    {
        type = Type::register32;
        dataP = new Register32(register32Map.find(lexeme)->second);
    }
    else if (registerSegmentMap.find(lexeme) != registerSegmentMap.end())
    {
        type = Type::registerSegment;
        dataP = new RegisterSegment(registerSegmentMap.find(lexeme)->second);
    }
    else if (sizeIdentifierMap.find(lexeme) != sizeIdentifierMap.end())
    {
        type = Type::sizeIdentifier;
        dataP = new SizeIdentifier(sizeIdentifierMap.find(lexeme)->second);
    }
    else if (dataIdentifierMap.find(lexeme) != dataIdentifierMap.end())
    {
        type = Type::dataIdentifier;
        dataP = new DataIdentifier(dataIdentifierMap.find(lexeme)->second);
    }
    else if (conditionMap.find(lexeme) != conditionMap.end())
    {
        type = Type::condition;
        dataP = new Condition(conditionMap.find(lexeme)->second);
    }
    else if (instructionMap.find(lexeme) != instructionMap.end())
    {
        type = Type::instruction;
        dataP = new Instruction(instructionMap.find(lexeme)->second);
    }
    else if (isCharQuoteCompatible(lexeme[0]))
    {
        type = Type::constantString;
        dataP = new string(lexeme.substr(1, lexeme.size() - 2));
    }
    else if (isCharNumberCompatible(lexeme[0]))
    {
        long long number;

        try
        {
            size_t stoiStop;
            uint realNumberSize;

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

        type = Type::constantNumber;
        dataP = new long long(number);
    }
    else
    {
        type = Type::userIdentifier;
        dataP = new string(lexeme);
    }
}

TSToken::TSToken(const TSToken &token)
{
    type = token.type;

    switch (type)
    {
        case Type::userIdentifier:
        case Type::constantString:
            dataP = new string(*((string *)token.dataP));
            break;
        case Type::singleChar:
            dataP = new SingleChar(*((SingleChar *)token.dataP));
            break;
        case Type::directive:
            dataP = new Directive(*((Directive *)token.dataP));
            break;
        case Type::instruction:
            dataP = new Instruction(*((Instruction *)token.dataP));
            break;
        case Type::register8:
            dataP = new Register8(*((Register8 *)token.dataP));
            break;
        case Type::register32:
            dataP = new Register32(*((Register32 *)token.dataP));
            break;
        case Type::registerSegment:
            dataP = new RegisterSegment(*((RegisterSegment *)token.dataP));
            break;
        case Type::sizeIdentifier:
            dataP = new SizeIdentifier(*((SizeIdentifier *)token.dataP));
            break;
        case Type::dataIdentifier:
            dataP = new DataIdentifier(*((DataIdentifier *)token.dataP));
            break;
        case Type::constantNumber:
            dataP = new long long(*((long long *)token.dataP));
            break;
        case Type::condition:
            dataP = new Condition(*((Condition *)token.dataP));
            break;
        default:
            break;
    }
}

TSToken &TSToken::operator=(const TSToken &token)
{
    switch (type)
    {
        case Type::userIdentifier:
        case Type::constantString:
            delete (string *)dataP;
            break;
        case Type::singleChar:
            delete (SingleChar *)dataP;
            break;
        case Type::directive:
            delete (Directive *)dataP;
            break;
        case Type::instruction:
            delete (Instruction *)dataP;
            break;
        case Type::register8:
            delete (Register8 *)dataP;
            break;
        case Type::register32:
            delete (Register32 *)dataP;
            break;
        case Type::registerSegment:
            delete (RegisterSegment *)dataP;
            break;
        case Type::sizeIdentifier:
            delete (SizeIdentifier *)dataP;
            break;
        case Type::dataIdentifier:
            delete (DataIdentifier *)dataP;
            break;
        case Type::constantNumber:
            delete (long long *)dataP;
            break;
        case Type::condition:
            delete (Condition *)dataP;
            break;
        default:
            break;
    }

    type = token.type;

    switch (type)
    {
        case Type::userIdentifier:
        case Type::constantString:
            dataP = new string(*((string *)token.dataP));
            break;
        case Type::singleChar:
            dataP = new SingleChar(*((SingleChar *)token.dataP));
            break;
        case Type::directive:
            dataP = new Directive(*((Directive *)token.dataP));
            break;
        case Type::instruction:
            dataP = new Instruction(*((Instruction *)token.dataP));
            break;
        case Type::register8:
            dataP = new Register8(*((Register8 *)token.dataP));
            break;
        case Type::register32:
            dataP = new Register32(*((Register32 *)token.dataP));
            break;
        case Type::registerSegment:
            dataP = new RegisterSegment(*((RegisterSegment *)token.dataP));
            break;
        case Type::sizeIdentifier:
            dataP = new SizeIdentifier(*((SizeIdentifier *)token.dataP));
            break;
        case Type::dataIdentifier:
            dataP = new DataIdentifier(*((DataIdentifier *)token.dataP));
            break;
        case Type::constantNumber:
            dataP = new long long(*((long long *)token.dataP));
            break;
        case Type::condition:
            dataP = new Condition(*((Condition *)token.dataP));
            break;
        default:
            break;
    }

    return *this;
}

TSToken::Type TSToken::getType() const
{
    return type;
}

string TSToken::stringValue() const
{
    return *((string *)dataP);
}

TSToken::SingleChar TSToken::singleCharValue() const
{
    return *((SingleChar *)dataP);
}

TSToken::Directive TSToken::directiveValue() const
{
    return *((Directive *)dataP);
}

TSToken::Instruction TSToken::instructionValue() const
{
    return *((Instruction *)dataP);
}

TSToken::Register8 TSToken::register8Value() const
{
    return *((Register8 *)dataP);
}

TSToken::Register32 TSToken::register32Value() const
{
    return *((Register32 *)dataP);
}

TSToken::RegisterSegment TSToken::registerSegmentValue() const
{
    return *((RegisterSegment *)dataP);
}

TSToken::SizeIdentifier TSToken::sizeIdentifierValue() const
{
    return *((SizeIdentifier *)dataP);
}

TSToken::DataIdentifier TSToken::dataIdentifierValue() const
{
    return *((DataIdentifier *)dataP);
}

long long TSToken::numberValue() const
{
    return *((long long *)dataP);
}

TSToken::Condition TSToken::conditionValue() const
{
    return *((Condition *)dataP);
}

TSToken::~TSToken()
{
    switch (type)
    {
        case Type::userIdentifier:
        case Type::constantString:
            delete (string *)dataP;
            break;
        case Type::singleChar:
            delete (SingleChar *)dataP;
            break;
        case Type::directive:
            delete (Directive *)dataP;
            break;
        case Type::instruction:
            delete (Instruction *)dataP;
            break;
        case Type::register8:
            delete (Register8 *)dataP;
            break;
        case Type::register32:
            delete (Register32 *)dataP;
            break;
        case Type::registerSegment:
            delete (RegisterSegment *)dataP;
            break;
        case Type::sizeIdentifier:
            delete (SizeIdentifier *)dataP;
            break;
        case Type::dataIdentifier:
            delete (DataIdentifier *)dataP;
            break;
        case Type::constantNumber:
            delete (long long *)dataP;
            break;
        case Type::condition:
            delete (Condition *)dataP;
            break;
        default:
            break;
    }
}

vector<TSTokenContainer> constructTokenContainerVector(vector<TSLexemeContainer> &lexemeContainerVector)
{
    vector<TSTokenContainer> tokenContainerVector;

    for (uint i = 0; i < lexemeContainerVector.size(); ++i)
    {
        tokenContainerVector.push_back({lexemeContainerVector[i].row,
                                        lexemeContainerVector[i].column,
                                        lexemeContainerVector[i].lexeme.size(),
                                        TSToken(lexemeContainerVector[i])});
    }

    return tokenContainerVector;
}
