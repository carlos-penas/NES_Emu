#ifndef CPUINSTRUCTION_H
#define CPUINSTRUCTION_H
#include "types.h"

class CPUInstruction
{
public:
    CPUInstruction();
    CPUInstruction(Byte opCode, int cycles, bool isJumpInstruction);
    CPUInstruction(Byte opCode, Byte data1, int cycles, bool isJumpInstruction);
    CPUInstruction(Byte opCode, Byte data1, Byte data2, int cycles, bool isJumpInstruction);

    Byte OpCode;
    Byte Data1;
    Byte Data2;

    int Cycles;
    int Bytes;

    bool IsDecoded;
    bool jumpInstruction;


};

#endif // CPUINSTRUCTION_H
