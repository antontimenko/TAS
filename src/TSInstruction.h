#ifndef _TSINSTRUCTION_H_
#define _TSINSTRUCTION_H_

#include "TSGlobal.h"
#include "TSOperandMask.h"
#include <bitset>

namespace TSOperandMask
{

extern const map<string, Mask> registerMap;

};

namespace TSInstruction
{

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

enum class DataIdentifier
{
    DB,
    DW,
    DD
};

extern const map<string, Instruction> instructionMap;
extern const map<string, DataIdentifier> dataIdentifierMap;

}

#endif
