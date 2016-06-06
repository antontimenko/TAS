#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include "Global.h"
#include "OperandMask.h"
#include "Integer.h"
#include <bitset>

namespace OperandMask {

extern const map<string, Mask> registerMap;

};

class InstructionSentence;

namespace InstructionNS {

enum class Instruction {
    DAA,
    NOT,
    PUSH,
    POP,
    OR,
    MOV,
    JBE
};

enum class DataIdentifier {
    DB,
    DW,
    DD
};

extern const map<string, Instruction> instructionMap;
extern const map<string, DataIdentifier> dataIdentifierMap;
extern const set<Instruction> jumpInstructionsSet;

class OperandFullMask {
public:
    inline OperandFullMask(OperandMask::Mask mask, Integer num) :
        mask(mask),
        num(num)
    {}

    inline OperandFullMask(OperandMask::Mask mask) :
        mask(mask)
    {}

    OperandMask::Mask mask;
    optional<Integer> num;
};

class Definition {
public:
    typedef InstructionNS::Instruction Instruction;

    inline Definition(vector<uchar> opcode,
                      uchar opcodeAdd,
                      Instruction inst,
                      vector<OperandFullMask> operandFullMasks,
                      function<vector<vector<uchar>>(Definition, InstructionSentence)> computeFunc) :
        opcode(opcode),
        opcodeAdd(opcodeAdd),
        instruction(inst),
        operandFullMasks(operandFullMasks),
        computeFunc(computeFunc)
    {}

    inline Definition(vector<uchar> opcode,
                      Instruction inst,
                      vector<OperandFullMask> operandFullMasks,
                      function<vector<vector<uchar>>(Definition, InstructionSentence)> computeFunc) :
        opcode(opcode),
        instruction(inst),
        operandFullMasks(operandFullMasks),
        computeFunc(computeFunc)
    {}

    vector<uchar> opcode;
    optional<uchar> opcodeAdd;
    Instruction instruction;
    vector<OperandFullMask> operandFullMasks;
    function<vector<vector<uchar>>(Definition, InstructionSentence)> computeFunc;
};

extern const vector<Definition> instructionDefinitionVector;

}

size_t getInstructionBytePresentSize(const vector<vector<uchar>> &instructionBytePresent);

#endif
