#include "TSInstruction.h"

#include "TSRawSentence.h"
#include "TSSentence.h"

namespace TSOperandMask
{

const map<string, Mask> registerMap = {
    {"AL", AL},
    {"CL", CL},
    {"DL", DL},
    {"BL", BL},
    {"AH", AH},
    {"CH", CH},
    {"DH", DH},
    {"BH", BH},
    
    {"AX", AX},
    {"CX", CX},
    {"DX", DX},
    {"BX", BX},
    {"SP", SP},
    {"BP", BP},
    {"SI", SI},
    {"DI", DI},

    {"EAX", EAX},
    {"ECX", ECX},
    {"EDX", EDX},
    {"EBX", EBX},
    {"ESP", ESP},
    {"EBP", EBP},
    {"ESI", ESI},
    {"EDI", EDI},

    {"ES", ES},
    {"CS", CS},
    {"SS", SS},
    {"DS", DS},
    {"FS", FS},
    {"GS", GS}
};

}

namespace TSInstruction
{

using namespace TSOperandMask;

const map<string, Instruction> instructionMap = {
    {"SCASD", Instruction::SCASD},
    {"RCL", Instruction::RCL},
    {"DIV", Instruction::DIV},
    {"OR", Instruction::OR},
    {"CMP", Instruction::CMP},
    {"AND", Instruction::AND},
    {"MOV", Instruction::MOV},
    {"ADD", Instruction::ADD},
    {"JNB", Instruction::JNB}
};

const map<string, DataIdentifier> dataIdentifierMap = {
    {"DB", DataIdentifier::DB},
    {"DW", DataIdentifier::DW},
    {"DD", DataIdentifier::DD}
};

vector<uchar> onlyOpcodeComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "onlyOpcodeComputeFunc" << endl;
    
    return {0, 0, 0};
}

vector<uchar> twoOpsOpcodeWithREGAndIMMComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "twoOpsWithREGAndImmComputeFunc" << endl;
    return {0, 0, 0};
}

vector<uchar> oneOpOpcodeWithREGComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "oneOpWithREGComputeFunc" << endl;
    return {0, 0, 0};
}

template<bool orderDirect>
vector<uchar> twoOpsClassicComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "twoOpsClassicComputeFunc";
    if (orderDirect)
        cout << "DirectOrder";
    else
        cout << "ReverseOrder";
    cout << endl;

    return {0, 0, 0};
}

vector<uchar> twoOpsAXSpecialWithIMMComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "twoOpsAXSpecialWithIMMComputeFunc" << endl;

    return {0, 0, 0};
}

template<bool orderDirect>
vector<uchar> twoOpsMoffsSpecialComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "twoOpsMoffsSpecialComputeFunc";
    if (orderDirect)
        cout << "DirectOrder";
    else
        cout << "ReverseOrder";
    cout << endl;

    return {0, 0, 0};
}

vector<uchar> twoOpsOpcodeIncWithImmComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "twoOpsOpcodeIncWithImmComputeFunc" << endl;

    return {0, 0, 0};
}

vector<uchar> relativeJumpComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    cout << "relativeJumpComputeFunc" << endl;

    return {0, 0, 0};
}

const vector<Definition> instructionDefinitionVector = {
    {{0xAF},    Instruction::SCASD, {},                                                               onlyOpcodeComputeFunc},

    {{0xC0}, 2, Instruction::RCL,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0xC1}, 2, Instruction::RCL,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8}},      twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0xD0}, 2, Instruction::RCL,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL, 1}},                         oneOpOpcodeWithREGComputeFunc},
    {{0xD1}, 2, Instruction::RCL,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8, 1}},   oneOpOpcodeWithREGComputeFunc},
    {{0xD0}, 2, Instruction::RCL,   {{UREG8_ANY | MEM8_ANY}, {CL}},                                   oneOpOpcodeWithREGComputeFunc},
    {{0xD3}, 2, Instruction::RCL,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {CL}},        oneOpOpcodeWithREGComputeFunc},
    
    {{0xF6}, 6, Instruction::DIV,   {{UREG8_ANY | MEM8_ANY}},                                         oneOpOpcodeWithREGComputeFunc},
    {{0xF7}, 6, Instruction::DIV,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}},              oneOpOpcodeWithREGComputeFunc},
    
    {{0x08},    Instruction::OR,    {{UREG8_ANY | MEM8_ANY}, {UREG8_ANY}},                            twoOpsClassicComputeFunc<true>},
    {{0x09},    Instruction::OR,    {{UREG16_ANY | MEM16_ANY}, {UREG16_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x09},    Instruction::OR,    {{UREG32_ANY | MEM32_ANY}, {UREG32_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x0A},    Instruction::OR,    {{UREG8_ANY}, {UREG8_ANY | MEM8_ANY}},                            twoOpsClassicComputeFunc<false>},
    {{0x0B},    Instruction::OR,    {{UREG16_ANY}, {UREG16_ANY | MEM16_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x0B},    Instruction::OR,    {{UREG32_ANY}, {UREG32_ANY | MEM32_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x0C},    Instruction::OR,    {{AL}, {IMM8_FILL}},                                              twoOpsAXSpecialWithIMMComputeFunc},
    {{0x0D},    Instruction::OR,    {{AX}, {IMM16_FILL}},                                             twoOpsAXSpecialWithIMMComputeFunc},
    {{0x0D},    Instruction::OR,    {{EAX}, {IMM32_FILL}},                                            twoOpsAXSpecialWithIMMComputeFunc},
    {{0x80}, 1, Instruction::OR,    {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 1, Instruction::OR,    {{UREG16_ANY | MEM16_ANY}, {IMM16_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 1, Instruction::OR,    {{UREG32_ANY | MEM32_ANY}, {IMM32_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x83}, 1, Instruction::OR,    {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8_FILL}}, twoOpsOpcodeWithREGAndIMMComputeFunc},

    
    {{0x38},    Instruction::CMP,   {{UREG8_ANY | MEM8_ANY}, {UREG8_ANY}},                            twoOpsClassicComputeFunc<true>},
    {{0x39},    Instruction::CMP,   {{UREG16_ANY | MEM16_ANY}, {UREG16_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x39},    Instruction::CMP,   {{UREG32_ANY | MEM32_ANY}, {UREG32_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x3A},    Instruction::CMP,   {{UREG8_ANY}, {UREG8_ANY | MEM8_ANY}},                            twoOpsClassicComputeFunc<false>},
    {{0x3B},    Instruction::CMP,   {{UREG16_ANY}, {UREG16_ANY | MEM16_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x3B},    Instruction::CMP,   {{UREG32_ANY}, {UREG32_ANY | MEM32_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x3C},    Instruction::CMP,   {{AL}, {IMM8_FILL}},                                              twoOpsAXSpecialWithIMMComputeFunc},
    {{0x3D},    Instruction::CMP,   {{AX}, {IMM16_FILL}},                                             twoOpsAXSpecialWithIMMComputeFunc},
    {{0x3D},    Instruction::CMP,   {{EAX}, {IMM32_FILL}},                                            twoOpsAXSpecialWithIMMComputeFunc},
    {{0x80}, 7, Instruction::CMP,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 7, Instruction::CMP,   {{UREG16_ANY | MEM16_ANY}, {IMM16_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 7, Instruction::CMP,   {{UREG32_ANY | MEM32_ANY}, {IMM32_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x83}, 7, Instruction::CMP,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8_FILL}}, twoOpsOpcodeWithREGAndIMMComputeFunc},

    {{0x20},    Instruction::AND,   {{UREG8_ANY | MEM8_ANY}, {UREG8_ANY}},                            twoOpsClassicComputeFunc<true>},
    {{0x21},    Instruction::AND,   {{UREG16_ANY | MEM16_ANY}, {UREG16_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x21},    Instruction::AND,   {{UREG32_ANY | MEM32_ANY}, {UREG32_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x22},    Instruction::AND,   {{UREG8_ANY}, {UREG8_ANY | MEM8_ANY}},                            twoOpsClassicComputeFunc<false>},
    {{0x23},    Instruction::AND,   {{UREG16_ANY}, {UREG16_ANY | MEM16_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x23},    Instruction::AND,   {{UREG32_ANY}, {UREG32_ANY | MEM32_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x24},    Instruction::AND,   {{AL}, {IMM8_FILL}},                                              twoOpsAXSpecialWithIMMComputeFunc},
    {{0x25},    Instruction::AND,   {{AX}, {IMM16_FILL}},                                             twoOpsAXSpecialWithIMMComputeFunc},
    {{0x25},    Instruction::AND,   {{EAX}, {IMM32_FILL}},                                            twoOpsAXSpecialWithIMMComputeFunc},
    {{0x80}, 4, Instruction::AND,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 4, Instruction::AND,   {{UREG32_ANY | MEM32_ANY}, {IMM16_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 4, Instruction::AND,   {{UREG32_ANY | MEM32_ANY}, {IMM32_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x83}, 4, Instruction::AND,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8_FILL}}, twoOpsOpcodeWithREGAndIMMComputeFunc},

    {{0x88},    Instruction::MOV,   {{UREG8_ANY | MEM8_ANY}, {UREG8_ANY}},                            twoOpsClassicComputeFunc<true>},
    {{0x89},    Instruction::MOV,   {{UREG16_ANY | MEM16_ANY}, {UREG16_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x89},    Instruction::MOV,   {{UREG32_ANY | MEM32_ANY}, {UREG32_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x8A},    Instruction::MOV,   {{UREG8_ANY}, {UREG8_ANY | MEM8_ANY}},                            twoOpsClassicComputeFunc<false>},
    {{0x8B},    Instruction::MOV,   {{UREG16_ANY}, {UREG16_ANY | MEM16_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x8B},    Instruction::MOV,   {{UREG32_ANY}, {UREG32_ANY | MEM32_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x8C},    Instruction::MOV,   {{UREG16_ANY | UREG32_ANY | MEM16_ANY}, {SREG_ANY}},              twoOpsClassicComputeFunc<true>},
    {{0x8E},    Instruction::MOV,   {{SREG_ANY}, {UREG16_ANY | MEM16_ANY}},                           twoOpsClassicComputeFunc<false>},
    {{0xA0},    Instruction::MOV,   {{AL}, {MEM_MODE_ANY}},                                           twoOpsMoffsSpecialComputeFunc<false>},
    {{0xA1},    Instruction::MOV,   {{AX | EAX}, {MEM_MODE_ANY}},                                     twoOpsMoffsSpecialComputeFunc<false>},
    {{0xA2},    Instruction::MOV,   {{MEM_MODE_ANY}, {AL}},                                           twoOpsMoffsSpecialComputeFunc<true>},
    {{0xA3},    Instruction::MOV,   {{MEM_MODE_ANY}, {AX | EAX}},                                     twoOpsMoffsSpecialComputeFunc<true>},
    {{0xB0},    Instruction::MOV,   {{UREG8_ANY}, {IMM8_FILL}},                                       twoOpsOpcodeIncWithImmComputeFunc},
    {{0xB8},    Instruction::MOV,   {{UREG16_ANY}, {IMM16_FILL}},                                     twoOpsOpcodeIncWithImmComputeFunc},
    {{0xB8},    Instruction::MOV,   {{UREG32_ANY}, {IMM32_FILL}},                                     twoOpsOpcodeIncWithImmComputeFunc},
    {{0xC6}, 0, Instruction::MOV,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0xC7}, 0, Instruction::MOV,   {{UREG16_ANY | MEM16_ANY}, {IMM16_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0xC7}, 0, Instruction::MOV,   {{UREG32_ANY | MEM32_ANY}, {IMM32_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},

    {{0x00},    Instruction::ADD,   {{UREG8_ANY | MEM8_ANY}, {UREG8_ANY}},                            twoOpsClassicComputeFunc<true>},
    {{0x01},    Instruction::ADD,   {{UREG16_ANY | MEM16_ANY}, {UREG16_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x01},    Instruction::ADD,   {{UREG32_ANY | MEM32_ANY}, {UREG32_ANY}},                         twoOpsClassicComputeFunc<true>},
    {{0x02},    Instruction::ADD,   {{UREG8_ANY}, {UREG8_ANY | MEM8_ANY}},                            twoOpsClassicComputeFunc<false>},
    {{0x03},    Instruction::ADD,   {{UREG16_ANY}, {UREG16_ANY | MEM16_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x03},    Instruction::ADD,   {{UREG32_ANY}, {UREG32_ANY | MEM32_ANY}},                         twoOpsClassicComputeFunc<false>},
    {{0x04},    Instruction::ADD,   {{AL}, {IMM8_FILL}},                                              twoOpsAXSpecialWithIMMComputeFunc},
    {{0x05},    Instruction::ADD,   {{AX}, {IMM16_FILL}},                                             twoOpsAXSpecialWithIMMComputeFunc},
    {{0x05},    Instruction::ADD,   {{EAX}, {IMM32_FILL}},                                            twoOpsAXSpecialWithIMMComputeFunc},
    {{0x80}, 0, Instruction::ADD,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 0, Instruction::ADD,   {{UREG16_ANY | MEM16_ANY}, {IMM16_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 0, Instruction::ADD,   {{UREG32_ANY | MEM32_ANY}, {IMM32_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x83}, 0, Instruction::ADD,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8_FILL}}, twoOpsOpcodeWithREGAndIMMComputeFunc},

    {{0x73},    Instruction::JNB,   {{MEM_MODE_ANY}},                                                 relativeJumpComputeFunc}
};

}
