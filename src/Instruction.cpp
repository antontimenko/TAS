#include "Instruction.h"

#include "Compiler.h"
#include "RawSentence.h"
#include "Sentence.h"
#include "Exception.h"

namespace OperandMask {

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

namespace InstructionNS {

using namespace OperandMask;

const map<string, Instruction> instructionMap = {
    {"DAA", Instruction::DAA},
    {"INC", Instruction::INC},
    {"DEC", Instruction::DEC},
    {"ADC", Instruction::ADC},
    {"CMP", Instruction::CMP},
    {"AND", Instruction::AND},
    {"MOV", Instruction::MOV},
    {"XOR", Instruction::XOR},
    {"JLE", Instruction::JLE}
};

const map<string, DataIdentifier> dataIdentifierMap = {
    {"DB", DataIdentifier::DB},
    {"DW", DataIdentifier::DW},
    {"DD", DataIdentifier::DD}
};

const set<Instruction> jumpInstructionsSet = {
    Instruction::JLE
};

uchar composeBits(bitset<2> one, bitset<3> two, bitset<3> three) {
    uchar res = 0;

    res |= three.to_ulong();
    res |= two.to_ulong() << 3;
    res |= one.to_ulong() << 6;

    return res;
}

bitset<3> getBitsetFromMask(const Mask &mask, bool useSecondRange = false) {
    uchar shift = useSecondRange ? 8 : 0;

    if (mask.match(OPVAL0 << shift))
        return 0b000;
    else if (mask.match(OPVAL1 << shift))
        return 0b001;
    else if (mask.match(OPVAL2 << shift))
        return 0b010;
    else if (mask.match(OPVAL3 << shift))
        return 0b011;
    else if (mask.match(OPVAL4 << shift))
        return 0b100;
    else if (mask.match(OPVAL5 << shift))
        return 0b101;
    else if (mask.match(OPVAL6 << shift))
        return 0b110;
    else if (mask.match(OPVAL7 << shift))
        return 0b111;
    else
        return 0b000;
}

bitset<2> getMOD(const InstructionSentence::OperandContainer &operandContainer) {
    const InstructionSentence::Operand &op = get<0>(operandContainer);

    if (!op.mask.match(MEM))
        return 0b11;

    if (op.isLinkable) {
        if (op.mask.match(MEM_16)) {
            if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= Integer::Size::S_16))
                return 0b10;
        } else {
            if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= Integer::Size::S_32))
                return 0b10;
        }
    }

    if (op.num == 0)
        return 0b00;

    if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= Integer::Size::S_8))
        return 0b01;

    if (op.mask.match(MEM_16)) {
        if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= Integer::Size::S_16))
            return 0b10;
    } else {
        if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= Integer::Size::S_32))
            return 0b10;
    }

    throw CompileError("disp too big", get<1>(operandContainer));
}

bitset<2> getScale(const InstructionSentence::Operand &op) {
    if (op.mask.match(MEM_32_INDEX_MULT1))
        return 0b00;
    else if (op.mask.match(MEM_32_INDEX_MULT2))
        return 0b01;
    else if (op.mask.match(MEM_32_INDEX_MULT4))
        return 0b10;
    else if (op.mask.match(MEM_32_INDEX_MULT8))
        return 0b11;
    else
        return 0b00;
}

Integer::Size getSuitableDispSize(const InstructionSentence::Operand &op) {
    if (op.isLinkable) {
        if (op.mask.match(MEM_32))
            return Integer::Size::S_32;
        else
            return Integer::Size::S_16;
    } else {
        if (op.num.sizeSigned() <= Integer::Size::S_8)
            return Integer::Size::S_8;
        else {
            if (op.mask.match(MEM_32))
                return Integer::Size::S_32;
            else
                return Integer::Size::S_16;
        }
    }
}

const map<InstructionSentence::SegmentPrefix, uchar> prefixMap = {
    {InstructionSentence::SegmentPrefix::ES, 0x26},
    {InstructionSentence::SegmentPrefix::CS, 0x2E},
    {InstructionSentence::SegmentPrefix::SS, 0x36},
    {InstructionSentence::SegmentPrefix::DS, 0x3E},
    {InstructionSentence::SegmentPrefix::FS, 0x64},
    {InstructionSentence::SegmentPrefix::GS, 0x65},
};

vector<vector<uchar>> generateOpcode(const vector<uchar> &opcodeByteVector) {
    vector<vector<uchar>> res;

    for (auto it = opcodeByteVector.begin(); it != opcodeByteVector.end(); ++it)
        res.push_back({*it});

    return res;
}

vector<vector<uchar>> generateSegmentOverridePrefixes(const InstructionSentence &instructionSentence, const InstructionSentence::Operand &op) {
    vector<vector<uchar>> res;

    if (op.mask.match(MEM)) {
        auto segmentPrefix = instructionSentence.segmentPrefix;

        if (op.mask.match(MEM_16)) {
            if ((!segmentPrefix) && (op.segName)) {
                auto assumeMap = instructionSentence.assume.getMap();
                auto assumeSegmentIt = assumeMap.find(*op.segName);
                if (assumeSegmentIt != assumeMap.end())
                    segmentPrefix = getSegmentOverridePrefix(assumeSegmentIt->second);
            }

            if (op.mask.match(MEM_16_BP) || 
                op.mask.match(MEM_16_BP_SI) ||
                op.mask.match(MEM_16_BP_DI))
            {
                if (segmentPrefix && (*segmentPrefix == InstructionSentence::SegmentPrefix::SS))
                    segmentPrefix = nullopt;
            } else {
                if (segmentPrefix && (*segmentPrefix == InstructionSentence::SegmentPrefix::DS))
                    segmentPrefix = nullopt;
            }
        } else {
            if (op.mask.match(MEM_32_BASE_EBP) ||
                op.mask.match(MEM_32_BASE_ESP))
            {
                if (segmentPrefix && (*segmentPrefix == InstructionSentence::SegmentPrefix::SS))
                    segmentPrefix = nullopt;
            } else {
                if (segmentPrefix && (*segmentPrefix == InstructionSentence::SegmentPrefix::DS))
                    segmentPrefix = nullopt;
            }
        }

        if (segmentPrefix)
            res.push_back({prefixMap.find(*segmentPrefix)->second});
    }

    return res;
}

constexpr uchar dataSizeOverridePrefix    = 0x66;
constexpr uchar addressSizeOverridePrefix = 0x67;

vector<vector<uchar>> generateAddressSizeOverridePrefix(const InstructionSentence::Operand &op) {
    vector<vector<uchar>> res;

    if ((op.mask.match(MEM)) &&
        (((Compiler::arch == Compiler::Arch::X86_16) &&
          (op.mask.match(MEM_32))) ||
         ((Compiler::arch == Compiler::Arch::X86_32) &&
          (op.mask.match(MEM_16)))))
    {
        res.push_back({addressSizeOverridePrefix});
    }

    return res;
}

vector<vector<uchar>> generateDataSizeOverridePrefix(const InstructionSentence::Operand &op) {
    vector<vector<uchar>> res;

    if (((Compiler::arch == Compiler::Arch::X86_16) &&
         (op.mask.match(S32))) ||
        ((Compiler::arch == Compiler::Arch::X86_32) &&
         (op.mask.match(S16))))
    {
        res.push_back({dataSizeOverridePrefix});
    }

    return res;
}

vector<vector<uchar>> generateMODRMAndSIB(const InstructionSentence::OperandContainer &opCont, const bitset<3> &reg) {
    vector<vector<uchar>> res;

    const InstructionSentence::Operand &op = get<0>(opCont);

    if (op.mask.match(MEM_16)) {
        if (op.mask.match(MEM_BASE)) {
            if (op.mask.match(MEM_16_BP) && (op.num == 0) && (!op.isLinkable)) {
                res.push_back({composeBits(0b01, reg, 0b110)});
                res.push_back({0});
            } else {
                res.push_back({composeBits(getMOD(opCont), reg, getBitsetFromMask(op.mask))});
                
                if ((op.num != 0) || op.isLinkable)
                    res.push_back(op.num.getCharArraySigned(getSuitableDispSize(op)));
            }
        } else {
            res.push_back({composeBits(0b00, reg, 0b110)});
            res.push_back({op.num.getCharArraySigned(Integer::Size::S_16)});
        }
    } else if (op.mask.match(MEM_32)) {
        if (op.mask.match(MEM_32_INDEX)) {
            bitset<2> scale = getScale(op);
            bitset<3> index = getBitsetFromMask(op.mask, true);

            if (op.mask.match(MEM_BASE)) {
                if (op.mask.match(MEM_32_BASE_EBP) && (op.num == 0) && (!op.isLinkable)) {
                    res.push_back({composeBits(0b01, reg, 0b100)});
                    res.push_back({composeBits(scale, index, 0b101)});
                    res.push_back({0});
                } else {
                    res.push_back({composeBits(getMOD(opCont), reg, 0b100)});
                    res.push_back({composeBits(scale, index, getBitsetFromMask(op.mask))});

                    if ((op.num != 0) || op.isLinkable)
                        res.push_back(op.num.getCharArraySigned(getSuitableDispSize(op)));
                }
            } else {
                res.push_back({composeBits(0b00, reg, 0b100)});
                res.push_back({composeBits(scale, index, 0b101)});
                res.push_back({op.num.getCharArraySigned(Integer::Size::S_32)});
            }
        } else {
            if (op.mask.match(MEM_BASE)) {
                if (op.mask.match(MEM_32_BASE_EBP) && (op.num == 0) && (!op.isLinkable)) {
                    res.push_back({composeBits(0b01, reg, 0b101)});
                    res.push_back({0});
                } else if (op.mask.match(MEM_32_BASE_ESP)) {
                    res.push_back({composeBits(getMOD(opCont), reg, 0b100)});
                    res.push_back({composeBits(0b00, 0b100, 0b100)});

                    if ((op.num != 0) || op.isLinkable)
                        res.push_back({op.num.getCharArraySigned(getSuitableDispSize(op))});
                } else {
                    res.push_back({composeBits(getMOD(opCont), reg, getBitsetFromMask(op.mask))});
                    
                    if ((op.num != 0) || op.isLinkable) 
                        res.push_back({op.num.getCharArraySigned(getSuitableDispSize(op))});
                }
            } else {
                res.push_back({composeBits(0b00, reg, 0b110)});
                res.push_back({op.num.getCharArraySigned(Integer::Size::S_32)});
            }
        }
    } else {
        res.push_back({composeBits(getMOD(opCont), reg, getBitsetFromMask(op.mask))});
    }

    return res;
}

vector<vector<uchar>> generateMODRMAndSIB(const InstructionSentence::OperandContainer &mainOpCont, const InstructionSentence::OperandContainer &regOpCont) {
    return generateMODRMAndSIB(mainOpCont, getBitsetFromMask(get<0>(regOpCont).mask));
}

vector<vector<uchar>> onlyOpcodeComputeFunc(Definition definition, InstructionSentence) {
    return generateOpcode(definition.opcode);
}

vector<vector<uchar>> twoOpsOpcodeWithREGAndIMMComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];
    auto secondOpCont = instructionSentence.operandContainerVector[1];

    auto &firstOp = get<0>(firstOpCont);
    auto &secondOp = get<0>(secondOpCont);

    auto segmentOverridePrefixes = generateSegmentOverridePrefixes(instructionSentence, firstOp);
    res.insert(res.end(), segmentOverridePrefixes.begin(), segmentOverridePrefixes.end());

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto addressSizeOverridePrefix = generateAddressSizeOverridePrefix(firstOp);
    res.insert(res.end(), addressSizeOverridePrefix.begin(), addressSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    auto addressMODRMAndSIB = generateMODRMAndSIB(firstOpCont, *definition.opcodeAdd);
    res.insert(res.end(), addressMODRMAndSIB.begin(), addressMODRMAndSIB.end());

    if (definition.operandFullMasks[1].mask.match(IMM32))
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_32));
    else if (definition.operandFullMasks[1].mask.match(IMM16))
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_16));
    else
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_8));

    return res;
}

vector<vector<uchar>> oneOpOpcodeWithREGComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];

    auto &firstOp = get<0>(firstOpCont);

    auto segmentOverridePrefixes = generateSegmentOverridePrefixes(instructionSentence, firstOp);
    res.insert(res.end(), segmentOverridePrefixes.begin(), segmentOverridePrefixes.end());

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto addressSizeOverridePrefix = generateAddressSizeOverridePrefix(firstOp);
    res.insert(res.end(), addressSizeOverridePrefix.begin(), addressSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    auto addressMODRMAndSIB = generateMODRMAndSIB(firstOpCont, *definition.opcodeAdd);
    res.insert(res.end(), addressMODRMAndSIB.begin(), addressMODRMAndSIB.end());

    return res;
}

template<bool orderDirect>
vector<vector<uchar>> twoOpsClassicComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    InstructionSentence::OperandContainer mainOpCont;
    InstructionSentence::OperandContainer regOpCont;
    
    if (orderDirect) {
        mainOpCont = instructionSentence.operandContainerVector[0];
        regOpCont = instructionSentence.operandContainerVector[1];
    } else {
        mainOpCont = instructionSentence.operandContainerVector[1];
        regOpCont = instructionSentence.operandContainerVector[0];
    }

    auto &mainOp = get<0>(mainOpCont);
    auto &regOp = get<0>(regOpCont);

    if (!mainOp.mask.matchAny(S_ANY))
        mainOp.mask |= regOp.mask & S_ANY;

    auto segmentOverridePrefixes = generateSegmentOverridePrefixes(instructionSentence, mainOp);
    res.insert(res.end(), segmentOverridePrefixes.begin(), segmentOverridePrefixes.end());

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(mainOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto addressSizeOverridePrefix = generateAddressSizeOverridePrefix(mainOp);
    res.insert(res.end(), addressSizeOverridePrefix.begin(), addressSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    auto addressMODRMAndSIB = generateMODRMAndSIB(mainOpCont, regOpCont);
    res.insert(res.end(), addressMODRMAndSIB.begin(), addressMODRMAndSIB.end());

    return res;
}

vector<vector<uchar>> twoOpsAXSpecialWithIMMComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];
    auto secondOpCont = instructionSentence.operandContainerVector[1];

    auto &firstOp = get<0>(firstOpCont);
    auto &secondOp = get<0>(secondOpCont);

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    if (definition.operandFullMasks[1].mask.match(IMM32))
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_32));
    else if (definition.operandFullMasks[1].mask.match(IMM16))
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_16));
    else
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_8));

    return res;
}

template<bool orderDirect>
vector<vector<uchar>> twoOpsMoffsSpecialComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    InstructionSentence::OperandContainer mainOpCont;
    InstructionSentence::OperandContainer moffsOpCont;
    
    if (orderDirect) {
        mainOpCont = instructionSentence.operandContainerVector[1];
        moffsOpCont = instructionSentence.operandContainerVector[0];
    } else {
        mainOpCont = instructionSentence.operandContainerVector[0];
        moffsOpCont = instructionSentence.operandContainerVector[1];
    }

    auto &mainOp = get<0>(mainOpCont);
    auto &moffsOp = get<0>(moffsOpCont);

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(mainOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    if (Compiler::arch == Compiler::Arch::X86_32)
        res.push_back(moffsOp.num.getCharArraySigned(Integer::Size::S_32));
    else
        res.push_back(moffsOp.num.getCharArraySigned(Integer::Size::S_16));
    
    return res;
}

vector<vector<uchar>> oneOpOpcodeIncComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];

    auto &firstOp = get<0>(firstOpCont);

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    uchar opcode = *(definition.opcode.end() - 1);
    opcode += getBitsetFromMask(firstOp.mask).to_ulong();
    res.push_back({opcode});

    return res;
}

vector<vector<uchar>> twoOpsOpcodeIncWithImmComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];
    auto secondOpCont = instructionSentence.operandContainerVector[1];

    auto &firstOp = get<0>(firstOpCont);
    auto &secondOp = get<0>(secondOpCont);

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    uchar opcode = *(definition.opcode.end() - 1);
    opcode += getBitsetFromMask(firstOp.mask).to_ulong();
    res.push_back({opcode});

    if (definition.operandFullMasks[1].mask.match(IMM32))
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_32));
    else if (definition.operandFullMasks[1].mask.match(IMM16))
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_16));
    else
        res.push_back(secondOp.num.getCharArrayAny(Integer::Size::S_8));

    return res;
}

vector<vector<uchar>> relativeJumpComputeFunc(Definition definition, InstructionSentence instructionSentence) {
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];

    auto &firstOp = get<0>(firstOpCont);

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    if (definition.operandFullMasks[0].mask.match(REL32)) {
        if (Compiler::arch == Compiler::Arch::X86_32)
            res.push_back(firstOp.num.getCharArraySigned(Integer::Size::S_32));
        else
            throw CompileError("too big relative path", get<1>(firstOpCont));
    } else if (definition.operandFullMasks[0].mask.match(REL16)) {
        if (Compiler::arch == Compiler::Arch::X86_32)
            res.push_back(firstOp.num.getCharArraySigned(Integer::Size::S_32));
        else
            res.push_back(firstOp.num.getCharArraySigned(Integer::Size::S_16));
    }
    else
        res.push_back(firstOp.num.getCharArraySigned(Integer::Size::S_8));

    return res;
}

const vector<Definition> instructionDefinitionVector = {
    {{0x27},    Instruction::DAA,   {},                                          onlyOpcodeComputeFunc},

    {{0xFE}, 0, Instruction::INC,   {{UREG8_ANY}},                               oneOpOpcodeWithREGComputeFunc},
    {{0x40},    Instruction::INC,   {{UREG16_ANY}},                              oneOpOpcodeIncComputeFunc},

    {{0xFE}, 1, Instruction::DEC,   {{MEM8_ANY}},                                oneOpOpcodeWithREGComputeFunc},
    {{0xFF}, 1, Instruction::DEC,   {{MEM16_ANY}},                               oneOpOpcodeWithREGComputeFunc},

    {{0x12},    Instruction::ADC,   {{UREG8_ANY}, {UREG8_ANY}},                  twoOpsClassicComputeFunc<false>},
    {{0x13},    Instruction::ADC,   {{UREG16_ANY}, {UREG16_ANY}},                twoOpsClassicComputeFunc<false>},

    {{0x3A},    Instruction::CMP,   {{UREG8_ANY}, {MEM8_ANY}},                   twoOpsClassicComputeFunc<false>},
    {{0x3B},    Instruction::CMP,   {{UREG16_ANY}, {MEM16_ANY}},                 twoOpsClassicComputeFunc<false>},

    {{0x20},    Instruction::AND,   {{MEM8_ANY}, {UREG8_ANY}},                   twoOpsClassicComputeFunc<true>},
    {{0x21},    Instruction::AND,   {{MEM16_ANY}, {UREG16_ANY}},                 twoOpsClassicComputeFunc<true>},

    {{0xB0},    Instruction::MOV,   {{UREG8_ANY}, {IMM8_FILL}},                  twoOpsOpcodeIncWithImmComputeFunc},
    {{0xB8},    Instruction::MOV,   {{UREG16_ANY}, {IMM16_FILL}},                twoOpsOpcodeIncWithImmComputeFunc},

    {{0x80}, 6, Instruction::XOR,   {{MEM8_ANY}, {IMM8_FILL}},                   twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x83}, 6, Instruction::XOR,   {{MEM16_ANY}, {IMM8_FILL}},                  twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0x81}, 6, Instruction::XOR,   {{MEM16_ANY}, {IMM16_FILL}},                 twoOpsOpcodeWithREGAndIMMComputeFunc},

    {{0x72},    Instruction::JLE,   {{REL8_FILL}},                               relativeJumpComputeFunc}
};

}

size_t getInstructionBytePresentSize(const vector<vector<uchar>> &instructionBytePresent) {
    size_t res = 0;

    for (auto it = instructionBytePresent.begin(); it != instructionBytePresent.end(); ++it)
        res += it->size();

    return res;
}
