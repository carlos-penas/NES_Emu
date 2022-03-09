#include "cpuinstruction.h"

CPUInstruction::CPUInstruction()
{
    IsDecoded = false;

    jumpInstruction = false;
}

CPUInstruction::CPUInstruction(uint8_t opCode, int cycles, bool isJumpInstruction)
{
    OpCode = opCode;
    Data1 = 0x00;
    Data2 = 0x00;

    Bytes = 1;
    Cycles = cycles;

    IsDecoded = true;

    jumpInstruction = isJumpInstruction;
}

CPUInstruction::CPUInstruction(uint8_t opCode, uint8_t data1, int cycles, bool isJumpInstruction)
{
    OpCode = opCode;
    Data1 = data1;
    Data2 = 0x00;

    Bytes = 2;
    Cycles = cycles;

    IsDecoded = true;

    jumpInstruction = isJumpInstruction;
}

CPUInstruction::CPUInstruction(uint8_t opCode, uint8_t data1, uint8_t data2, int cycles, bool isJumpInstruction)
{
    OpCode = opCode;
    Data1 = data1;
    Data2 = data2;

    Bytes = 3;
    Cycles = cycles;

    IsDecoded = true;

    jumpInstruction = isJumpInstruction;
}
