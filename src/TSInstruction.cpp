#include "TSInstruction.h"

#include "TSCompiler.h"
#include "TSRawSentence.h"
#include "TSSentence.h"
#include "TSException.h"

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

const set<Instruction> jumpInstructionsSet = {
    Instruction::JNB
};

uchar composeBits(bitset<2> one, bitset<3> two, bitset<3> three)
{
    uchar res = 0;

    res |= three.to_ulong();
    res |= two.to_ulong() << 3;
    res |= one.to_ulong() << 6;

    return res;
}

bitset<3> getBitsetFromMask(const Mask &mask, bool useSecondRange = false)
{
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

bitset<2> getMOD(const TSInstructionSentence::OperandContainer &operandContainer)
{
    const TSInstructionSentence::Operand &op = get<0>(operandContainer);

    if (!op.mask.match(MEM))
        return 0b11;

    if (op.num == 0)
        return 0b00;

    if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= TSInteger::Size::S_8))
        return 0b01;

    if (op.mask.match(MEM_16))
    {
        if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= TSInteger::Size::S_16))
            return 0b10;
    }
    else
    {
        if ((op.num.sizeSigned()) && (*op.num.sizeSigned() <= TSInteger::Size::S_32))
            return 0b10;
    }

    throw TSCompileError("disp too big", get<1>(operandContainer));
}

bitset<2> getScale(const TSInstructionSentence::Operand &op)
{
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

TSInteger::Size getSuitableDispSize(const TSInstructionSentence::Operand &op)
{
    if (op.num.sizeSigned() <= TSInteger::Size::S_8)
        return TSInteger::Size::S_8;
    else
    {
        if (op.mask.match(MEM_32))
            return TSInteger::Size::S_32;
        else
            return TSInteger::Size::S_16;
    }
}

const map<TSInstructionSentence::Prefix, uchar> prefixMap = {
    {TSInstructionSentence::Prefix::ES, 0x26},
    {TSInstructionSentence::Prefix::CS, 0x2E},
    {TSInstructionSentence::Prefix::SS, 0x36},
    {TSInstructionSentence::Prefix::DS, 0x3E},
    {TSInstructionSentence::Prefix::FS, 0x64},
    {TSInstructionSentence::Prefix::GS, 0x65},
};

vector<vector<uchar>> generateOpcode(const vector<uchar> &opcodeByteVector)
{
    vector<vector<uchar>> res;

    for (auto it = opcodeByteVector.begin(); it != opcodeByteVector.end(); ++it)
        res.push_back({*it});

    return res;
}

vector<vector<uchar>> generateSegmentOverridePrefixes(const TSInstructionSentence &instructionSentence)
{
    vector<vector<uchar>> res;

    for (auto it = instructionSentence.prefixVector.begin(); it != instructionSentence.prefixVector.end(); ++it)
        res.push_back({prefixMap.find(*it)->second});

    return res;
}

constexpr uchar dataSizeOverridePrefix    = 0x66;
constexpr uchar addressSizeOverridePrefix = 0x67;

vector<vector<uchar>> generateAddressSizeOverridePrefix(const TSInstructionSentence::Operand &op)
{
    vector<vector<uchar>> res;

    if ((op.mask.match(MEM)) &&
        (((TSCompiler::arch == TSCompiler::Arch::X86_16) &&
          (op.mask.match(MEM_32))) ||
         ((TSCompiler::arch == TSCompiler::Arch::X86_32) &&
          (op.mask.match(MEM_16)))))
    {
        res.push_back({addressSizeOverridePrefix});
    }

    return res;
}

vector<vector<uchar>> generateDataSizeOverridePrefix(const TSInstructionSentence::Operand &op)
{
    vector<vector<uchar>> res;

    if (((TSCompiler::arch == TSCompiler::Arch::X86_16) &&
         (op.mask.match(S32))) ||
        ((TSCompiler::arch == TSCompiler::Arch::X86_32) &&
         (op.mask.match(S16))))
    {
        res.push_back({dataSizeOverridePrefix});
    }

    return res;
}

vector<vector<uchar>> generateMODRMAndSIB(const TSInstructionSentence::OperandContainer &opCont, const bitset<3> &reg)
{
    vector<vector<uchar>> res;

    const TSInstructionSentence::Operand &op = get<0>(opCont);

    if (op.mask.match(MEM_16))
    {
        if (op.mask.match(MEM_BASE))
        {
            if (op.mask.match(MEM_16_BP) && (op.num == 0))
            {
                res.push_back({composeBits(0b01, reg, 0b110)});
                res.push_back({0});
            }
            else
            {
                res.push_back({composeBits(getMOD(opCont), reg, getBitsetFromMask(op.mask))});
                
                if (op.num != 0)
                    res.push_back(op.num.getCharArraySigned(getSuitableDispSize(op)));
            }
        }
        else
        {
            res.push_back({composeBits(0b00, reg, 0b110)});
            res.push_back({op.num.getCharArraySigned(TSInteger::Size::S_16)});
        }
    }
    else if (op.mask.match(MEM_32))
    {
        if (op.mask.match(MEM_32_INDEX))
        {
            bitset<2> scale = getScale(op);
            bitset<3> index = getBitsetFromMask(op.mask, true);

            if (op.mask.match(MEM_BASE))
            {
                if (op.mask.match(MEM_32_BASE_EBP) && (op.num == 0))
                {
                    res.push_back({composeBits(0b01, reg, 0b100)});
                    res.push_back({composeBits(scale, index, 0b101)});
                    res.push_back({0});
                }
                else
                {
                    res.push_back({composeBits(getMOD(opCont), reg, 0b100)});
                    res.push_back({composeBits(scale, index, getBitsetFromMask(op.mask))});

                    if (op.num != 0)
                        res.push_back(op.num.getCharArraySigned(getSuitableDispSize(op)));
                }
            }
            else
            {
                res.push_back({composeBits(0b00, reg, 0b100)});
                res.push_back({composeBits(scale, index, 0b101)});
                res.push_back({op.num.getCharArraySigned(TSInteger::Size::S_32)});
            }
        }
        else
        {
            if (op.mask.match(MEM_BASE))
            {
                if (op.mask.match(MEM_32_BASE_EBP) && (op.num == 0))
                {
                    res.push_back({composeBits(0b01, reg, 0b101)});
                    res.push_back({0});
                }
                else if (op.mask.match(MEM_32_BASE_ESP))
                {
                    res.push_back({composeBits(getMOD(opCont), reg, 0b100)});
                    res.push_back({composeBits(0b00, 0b100, 0b100)});

                    if (op.num != 0)
                        res.push_back({op.num.getCharArraySigned(getSuitableDispSize(op))});
                }
                else
                {
                    res.push_back({composeBits(getMOD(opCont), reg, getBitsetFromMask(op.mask))});
                    
                    if (op.num != 0) 
                        res.push_back({op.num.getCharArraySigned(getSuitableDispSize(op))});
                }
            }
            else
            {
                res.push_back({composeBits(0b00, reg, 0b110)});
                res.push_back({op.num.getCharArraySigned(TSInteger::Size::S_32)});
            }
        }
    }
    else
    {
        res.push_back({composeBits(getMOD(opCont), reg, getBitsetFromMask(op.mask))});
    }

    return res;
}

vector<vector<uchar>> generateMODRMAndSIB(const TSInstructionSentence::OperandContainer &mainOpCont, const TSInstructionSentence::OperandContainer &regOpCont)
{
    return generateMODRMAndSIB(mainOpCont, getBitsetFromMask(get<0>(regOpCont).mask));
}

vector<vector<uchar>> onlyOpcodeComputeFunc(Definition definition, TSInstructionSentence)
{
    return generateOpcode(definition.opcode);
}

vector<vector<uchar>> twoOpsOpcodeWithREGAndIMMComputeFunc(Definition definition, TSInstructionSentence instructionSentence)
{
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];
    auto secondOpCont = instructionSentence.operandContainerVector[1];

    auto &firstOp = get<0>(firstOpCont);
    auto &secondOp = get<0>(secondOpCont);

    auto segmentOverridePrefixes = generateSegmentOverridePrefixes(instructionSentence);
    res.insert(res.end(), segmentOverridePrefixes.begin(), segmentOverridePrefixes.end());

    auto addressSizeOverridePrefix = generateAddressSizeOverridePrefix(firstOp);
    res.insert(res.end(), addressSizeOverridePrefix.begin(), addressSizeOverridePrefix.end());

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    auto addressMODRMAndSIB = generateMODRMAndSIB(firstOpCont, *definition.opcodeAdd);
    res.insert(res.end(), addressMODRMAndSIB.begin(), addressMODRMAndSIB.end());

    if (definition.operandFullMasks[1].mask.match(IMM32))
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_32));
    else if (definition.operandFullMasks[1].mask.match(IMM16))
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_16));
    else
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_8));

    return res;
}

vector<vector<uchar>> oneOpOpcodeWithREGComputeFunc(Definition definition, TSInstructionSentence instructionSentence)
{
    vector<vector<uchar>> res;

    auto firstOpCont = instructionSentence.operandContainerVector[0];

    auto &firstOp = get<0>(firstOpCont);

    auto segmentOverridePrefixes = generateSegmentOverridePrefixes(instructionSentence);
    res.insert(res.end(), segmentOverridePrefixes.begin(), segmentOverridePrefixes.end());

    auto addressSizeOverridePrefix = generateAddressSizeOverridePrefix(firstOp);
    res.insert(res.end(), addressSizeOverridePrefix.begin(), addressSizeOverridePrefix.end());

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(firstOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    auto addressMODRMAndSIB = generateMODRMAndSIB(firstOpCont, *definition.opcodeAdd);
    res.insert(res.end(), addressMODRMAndSIB.begin(), addressMODRMAndSIB.end());

    return res;
}

template<bool orderDirect>
vector<vector<uchar>> twoOpsClassicComputeFunc(Definition definition, TSInstructionSentence instructionSentence)
{
    vector<vector<uchar>> res;

    TSInstructionSentence::OperandContainer mainOpCont;
    TSInstructionSentence::OperandContainer regOpCont;
    
    if (orderDirect)
    {
        mainOpCont = instructionSentence.operandContainerVector[0];
        regOpCont = instructionSentence.operandContainerVector[1];
    }
    else
    {
        mainOpCont = instructionSentence.operandContainerVector[1];
        regOpCont = instructionSentence.operandContainerVector[0];
    }

    auto &mainOp = get<0>(mainOpCont);
    auto &regOp = get<0>(regOpCont);

    if (!mainOp.mask.matchAny(S_ANY))
        mainOp.mask |= regOp.mask & S_ANY;

    auto segmentOverridePrefixes = generateSegmentOverridePrefixes(instructionSentence);
    res.insert(res.end(), segmentOverridePrefixes.begin(), segmentOverridePrefixes.end());

    auto addressSizeOverridePrefix = generateAddressSizeOverridePrefix(mainOp);
    res.insert(res.end(), addressSizeOverridePrefix.begin(), addressSizeOverridePrefix.end());

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(mainOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    auto addressMODRMAndSIB = generateMODRMAndSIB(mainOpCont, regOpCont);
    res.insert(res.end(), addressMODRMAndSIB.begin(), addressMODRMAndSIB.end());

    return res;
}

vector<vector<uchar>> twoOpsAXSpecialWithIMMComputeFunc(Definition definition, TSInstructionSentence instructionSentence)
{
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
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_32));
    else if (definition.operandFullMasks[1].mask.match(IMM16))
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_16));
    else
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_8));

    return res;
}

template<bool orderDirect>
vector<vector<uchar>> twoOpsMoffsSpecialComputeFunc(Definition definition, TSInstructionSentence instructionSentence)
{
    vector<vector<uchar>> res;

    TSInstructionSentence::OperandContainer mainOpCont;
    TSInstructionSentence::OperandContainer moffsOpCont;
    
    if (orderDirect)
    {
        mainOpCont = instructionSentence.operandContainerVector[1];
        moffsOpCont = instructionSentence.operandContainerVector[0];
    }
    else
    {
        mainOpCont = instructionSentence.operandContainerVector[0];
        moffsOpCont = instructionSentence.operandContainerVector[1];
    }

    auto &mainOp = get<0>(mainOpCont);
    auto &moffsOp = get<0>(moffsOpCont);

    auto dataSizeOverridePrefix = generateDataSizeOverridePrefix(mainOp);
    res.insert(res.end(), dataSizeOverridePrefix.begin(), dataSizeOverridePrefix.end());

    auto opcode = generateOpcode(definition.opcode);
    res.insert(res.end(), opcode.begin(), opcode.end());

    if (TSCompiler::arch == TSCompiler::Arch::X86_32)
        res.push_back(moffsOp.num.getCharArraySigned(TSInteger::Size::S_32));
    else
        res.push_back(moffsOp.num.getCharArraySigned(TSInteger::Size::S_16));
    
    return res;
}

vector<vector<uchar>> twoOpsOpcodeIncWithImmComputeFunc(Definition definition, TSInstructionSentence instructionSentence)
{
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
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_32));
    else if (definition.operandFullMasks[1].mask.match(IMM16))
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_16));
    else
        res.push_back(secondOp.num.getCharArrayAny(TSInteger::Size::S_8));

    return res;
}

vector<vector<uchar>> relativeJumpComputeFunc([[gnu::unused]] Definition definition, [[gnu::unused]] TSInstructionSentence instructionSentence)
{
    //TODO: Implement relative jump

    return {{0}, {0}, {0}};
}

const vector<Definition> instructionDefinitionVector = {
    {{0xAF},    Instruction::SCASD, {},                                                               onlyOpcodeComputeFunc},

    {{0xC0}, 2, Instruction::RCL,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL}},                            twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0xC1}, 2, Instruction::RCL,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8}},      twoOpsOpcodeWithREGAndIMMComputeFunc},
    {{0xD0}, 2, Instruction::RCL,   {{UREG8_ANY | MEM8_ANY}, {IMM8_FILL, 1}},                         oneOpOpcodeWithREGComputeFunc},
    {{0xD1}, 2, Instruction::RCL,   {{UREG16_ANY | MEM16_ANY | UREG32_ANY | MEM32_ANY}, {IMM8, 1}},   oneOpOpcodeWithREGComputeFunc},
    {{0xD2}, 2, Instruction::RCL,   {{UREG8_ANY | MEM8_ANY}, {CL}},                                   oneOpOpcodeWithREGComputeFunc},
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
    {{0x81}, 4, Instruction::AND,   {{UREG16_ANY | MEM16_ANY}, {IMM16_FILL}},                         twoOpsOpcodeWithREGAndIMMComputeFunc},
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
    {{0xA0},    Instruction::MOV,   {{AL}, {MEM_MODE_ANY | S_ANY}},                                   twoOpsMoffsSpecialComputeFunc<false>},
    {{0xA1},    Instruction::MOV,   {{AX | EAX}, {MEM_MODE_ANY | S_ANY}},                             twoOpsMoffsSpecialComputeFunc<false>},
    {{0xA2},    Instruction::MOV,   {{MEM_MODE_ANY | S_ANY}, {AL}},                                   twoOpsMoffsSpecialComputeFunc<true>},
    {{0xA3},    Instruction::MOV,   {{MEM_MODE_ANY | S_ANY}, {AX | EAX}},                             twoOpsMoffsSpecialComputeFunc<true>},
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

size_t getInstructionBytePresentSize(const vector<vector<uchar>> &instructionBytePresent)
{
    size_t res = 0;

    for (auto it = instructionBytePresent.begin(); it != instructionBytePresent.end(); ++it)
        res += it->size();

    return res;
}
