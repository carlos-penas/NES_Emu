#ifndef CPUINSTRUCTION_H
#define CPUINSTRUCTION_H
#include "types.h"
#include <string>

class CPUInstruction
{
public:
    CPUInstruction();
    CPUInstruction(Byte opCode, int cycles, bool isJumpInstruction, std::string name);
    CPUInstruction(Byte opCode, Byte data1, int cycles, bool isJumpInstruction, std::string name);
    CPUInstruction(Byte opCode, Byte data1, Byte data2, int cycles, bool isJumpInstruction, std::string name);

    Byte OpCode;
    Byte Data1;
    Byte Data2;

    int Cycles;
    int Bytes;

    std::string Name;

    bool isExecuting;
    bool jumpInstruction;


};

#endif // CPUINSTRUCTION_H
