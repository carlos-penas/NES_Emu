#ifndef CPUINSTRUCTION_H
#define CPUINSTRUCTION_H
#include "types.h"
#include <QString>

class CPUInstruction
{
public:
    CPUInstruction();
    CPUInstruction(Byte opCode, int cycles, bool isJumpInstruction, QString name);
    CPUInstruction(Byte opCode, Byte data1, int cycles, bool isJumpInstruction, QString name);
    CPUInstruction(Byte opCode, Byte data1, Byte data2, int cycles, bool isJumpInstruction, QString name);

    Byte OpCode;
    Byte Data1;
    Byte Data2;

    int Cycles;
    int Bytes;

    QString Name;

    bool isExecuting;
    bool jumpInstruction;


};

#endif // CPUINSTRUCTION_H
