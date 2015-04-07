#include "TSInstruction.h"

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

}
