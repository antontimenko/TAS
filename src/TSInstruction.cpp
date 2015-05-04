#include "TSInstruction.h"

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

}
