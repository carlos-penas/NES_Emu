#include "cpuinstruction.h"

CPUInstruction::CPUInstruction()
{
    isExecuting = false;
    jumpInstruction = false;
}

CPUInstruction::CPUInstruction(Byte opCode, int cycles, bool isJumpInstruction, std::string name)
{
    OpCode = opCode;
    Data1 = 0x00;
    Data2 = 0x00;

    Bytes = 1;
    Cycles = cycles;

    isExecuting = false;

    jumpInstruction = isJumpInstruction;

    Name = name;
}

CPUInstruction::CPUInstruction(Byte opCode, Byte data1, int cycles, bool isJumpInstruction, std::string name)
{
    OpCode = opCode;
    Data1 = data1;
    Data2 = 0x00;

    Bytes = 2;
    Cycles = cycles;

    isExecuting = false;

    jumpInstruction = isJumpInstruction;

    Name = name;
}

CPUInstruction::CPUInstruction(Byte opCode, Byte data1, Byte data2, int cycles, bool isJumpInstruction, std::string name)
{
    OpCode = opCode;
    Data1 = data1;
    Data2 = data2;

    Bytes = 3;
    Cycles = cycles;

    isExecuting = false;

    jumpInstruction = isJumpInstruction;

    Name = name;
}
