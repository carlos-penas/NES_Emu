#ifndef CPUINSTRUCTION_H
#define CPUINSTRUCTION_H
#include <cstdint>

class CPUInstruction
{
public:
    CPUInstruction();
    CPUInstruction(uint8_t opCode, int cycles, bool isJumpInstruction);
    CPUInstruction(uint8_t opCode, uint8_t data1, int cycles, bool isJumpInstruction);
    CPUInstruction(uint8_t opCode, uint8_t data1, uint8_t data2, int cycles, bool isJumpInstruction);

    uint8_t OpCode;
    uint8_t Data1;
    uint8_t Data2;

    int Cycles;
    int Bytes;

    bool IsDecoded;
    bool jumpInstruction;


};

#endif // CPUINSTRUCTION_H
