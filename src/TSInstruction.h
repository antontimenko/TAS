#ifndef _TSINSTRUCTION_H_
#define _TSINSTRUCTION_H_

#include "TSGlobal.h"
#include "TSOperandMask.h"
#include "TSInteger.h"
#include <initializer_list>
#include <bitset>

namespace TSOperandMask
{

extern const map<string, Mask> registerMap;

};

class TSInstructionSentence;

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
extern const set<Instruction> jumpInstructionsSet;

class OperandFullMask
{
public:
    inline OperandFullMask(TSOperandMask::Mask mask, TSInteger num) :
        mask(mask),
        num(num)
    {}

    inline OperandFullMask(TSOperandMask::Mask mask) :
        mask(mask)
    {}

    TSOperandMask::Mask mask;
    optional<TSInteger> num;
};

class Definition
{
public:
    typedef TSInstruction::Instruction Instruction;

    inline Definition(vector<uchar> opcode,
                      uchar opcodeAdd,
                      Instruction inst,
                      vector<OperandFullMask> operandFullMasks,
                      function<vector<vector<uchar>>(Definition, TSInstructionSentence)> computeFunc) :
        opcode(opcode),
        opcodeAdd(opcodeAdd),
        instruction(inst),
        operandFullMasks(operandFullMasks),
        computeFunc(computeFunc)
    {}

    inline Definition(vector<uchar> opcode,
                      Instruction inst,
                      vector<OperandFullMask> operandFullMasks,
                      function<vector<vector<uchar>>(Definition, TSInstructionSentence)> computeFunc) :
        opcode(opcode),
        instruction(inst),
        operandFullMasks(operandFullMasks),
        computeFunc(computeFunc)
    {}

    vector<uchar> opcode;
    optional<uchar> opcodeAdd;
    Instruction instruction;
    vector<OperandFullMask> operandFullMasks;
    function<vector<vector<uchar>>(Definition, TSInstructionSentence)> computeFunc;
};

extern const vector<Definition> instructionDefinitionVector;

}

size_t getInstructionBytePresentSize(const vector<vector<uchar>> &instructionBytePresent);

#endif
