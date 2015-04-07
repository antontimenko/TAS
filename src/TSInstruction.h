#ifndef _TSINSTRUCTION_H_
#define _TSINSTRUCTION_H_

#include "TSGlobal.h"
#include <stdint.h>

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

extern const map<string, Instruction> instructionMap;

class Operand
{
    /**
     * This enum is used to determine needed instruction by operands.
     * First four bytes is used to encode general operand type,
     * particularly first two for registers, third for memory and fourth for constants.
     * The rest bytes used to encode particular type.
     */
    enum Type : uint64_t
    {
        REG_8_BIT               = 1ULL << 0,
        REG_16_BIT              = 1ULL << 1,
        REG_32_BIT              = 1ULL << 2,
        REG_64_BIT              = 1ULL << 3,
        REG_8_BIT_HIGH          = 1ULL << 4,
        REG_SEG                 = 1ULL << 5,
        MEM_16_BIT              = 1ULL << 16,
        MEM_32_BIT              = 1ULL << 17,
        MEM_64_BIT              = 1ULL << 18,
        IMM_8_BIT               = 1ULL << 31,
        IMM_16_BIT              = (1ULL << 30) | IMM_8_BIT,
        IMM_32_BIT              = (1ULL << 29) | IMM_16_BIT,
        IMM_64_BIT              = (1ULL << 28) | IMM_32_BIT,
        
        REG_0                   = 1ULL << 32,
        REG_1                   = 1ULL << 33,
        REG_2                   = 1ULL << 34,
        REG_3                   = 1ULL << 35,
        REG_4                   = 1ULL << 36,
        REG_5                   = 1ULL << 37,
        REG_6                   = 1ULL << 38,
        REG_7                   = 1ULL << 39,
        REG_8                   = 1ULL << 40,
        REG_9                   = 1ULL << 41,
        REG_10                  = 1ULL << 42,
        REG_11                  = 1ULL << 43,
        REG_12                  = 1ULL << 44,
        REG_13                  = 1ULL << 45,
        REG_14                  = 1ULL << 46,
        REG_15                  = 1ULL << 47,
        
        REG_AL                  = REG_8_BIT | REG_0,
        REG_CL                  = REG_8_BIT | REG_1,
        REG_DL                  = REG_8_BIT | REG_2,
        REG_BL                  = REG_8_BIT | REG_3,
        REG_AH                  = REG_8_BIT | REG_0 | REG_8_BIT_HIGH,
        REG_CH                  = REG_8_BIT | REG_1 | REG_8_BIT_HIGH,
        REG_DH                  = REG_8_BIT | REG_2 | REG_8_BIT_HIGH,
        REG_BH                  = REG_8_BIT | REG_3 | REG_8_BIT_HIGH,
        REG_SPL                 = REG_8_BIT | REG_4,
        REG_BPL                 = REG_8_BIT | REG_5,
        REG_SIL                 = REG_8_BIT | REG_6,
        REG_DIL                 = REG_8_BIT | REG_7,
        REG_R8L                 = REG_8_BIT | REG_8,
        REG_R9L                 = REG_8_BIT | REG_9,
        REG_R10L                = REG_8_BIT | REG_10,
        REG_R11L                = REG_8_BIT | REG_11,
        REG_R12L                = REG_8_BIT | REG_12,
        REG_R13L                = REG_8_BIT | REG_13,
        REG_R14L                = REG_8_BIT | REG_14,
        REG_R15L                = REG_8_BIT | REG_15,
        
        REG_AX                  = REG_16_BIT | REG_0,
        REG_CX                  = REG_16_BIT | REG_1,
        REG_DX                  = REG_16_BIT | REG_2,
        REG_BX                  = REG_16_BIT | REG_3,
        REG_SP                  = REG_16_BIT | REG_4,
        REG_BP                  = REG_16_BIT | REG_5,
        REG_SI                  = REG_16_BIT | REG_6,
        REG_DI                  = REG_16_BIT | REG_7,
        REG_R8W                 = REG_16_BIT | REG_8,
        REG_R9W                 = REG_16_BIT | REG_9,
        REG_R10W                = REG_16_BIT | REG_10,
        REG_R11W                = REG_16_BIT | REG_11,
        REG_R12W                = REG_16_BIT | REG_12,
        REG_R13W                = REG_16_BIT | REG_13,
        REG_R14W                = REG_16_BIT | REG_14,
        REG_R15W                = REG_16_BIT | REG_15,
        
        REG_EAX                 = REG_32_BIT | REG_0,
        REG_ECX                 = REG_32_BIT | REG_1,
        REG_EDX                 = REG_32_BIT | REG_2,
        REG_EBX                 = REG_32_BIT | REG_3,
        REG_ESP                 = REG_32_BIT | REG_4,
        REG_EBP                 = REG_32_BIT | REG_5,
        REG_ESI                 = REG_32_BIT | REG_6,
        REG_EDI                 = REG_32_BIT | REG_7,
        REG_R8D                 = REG_32_BIT | REG_8,
        REG_R9D                 = REG_32_BIT | REG_9,
        REG_R10D                = REG_32_BIT | REG_10,
        REG_R11D                = REG_32_BIT | REG_11,
        REG_R12D                = REG_32_BIT | REG_12,
        REG_R13D                = REG_32_BIT | REG_13,
        REG_R14D                = REG_32_BIT | REG_14,
        REG_R15D                = REG_32_BIT | REG_15,
        
        REG_RAX                 = REG_64_BIT | REG_0,
        REG_RCX                 = REG_64_BIT | REG_1,
        REG_RDX                 = REG_64_BIT | REG_2,
        REG_RBX                 = REG_64_BIT | REG_3,
        REG_RSP                 = REG_64_BIT | REG_4,
        REG_RBP                 = REG_64_BIT | REG_5,
        REG_RSI                 = REG_64_BIT | REG_6,
        REG_RDI                 = REG_64_BIT | REG_7,
        REG_R8                  = REG_64_BIT | REG_8,
        REG_R9                  = REG_64_BIT | REG_9,
        REG_R10                 = REG_64_BIT | REG_10,
        REG_R11                 = REG_64_BIT | REG_11,
        REG_R12                 = REG_64_BIT | REG_12,
        REG_R13                 = REG_64_BIT | REG_13,
        REG_R14                 = REG_64_BIT | REG_14,
        REG_R15                 = REG_64_BIT | REG_15,

        REG_ES                  = REG_SEG | REG_0,
        REG_CS                  = REG_SEG | REG_1,
        REG_SS                  = REG_SEG | REG_2,
        REG_DS                  = REG_SEG | REG_3,
        REG_FS                  = REG_SEG | REG_4,
        REG_GS                  = REG_SEG | REG_5,

        MEM_DIRECT              = 1ULL << 32,
        MEM_BASE                = 1ULL << 33,
        MEM_BASE_DISP           = 1ULL << 34,
        MEM_BASE_INDEX          = 1ULL << 35,
        MEM_BASE_INDEX_DISP     = 1ULL << 36,
    };
};

}

#endif
