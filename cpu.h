#ifndef CPU_H
#define CPU_H
#include <cstdint>
#include <cpuinstruction.h>

typedef uint8_t register8;
typedef uint16_t register16;


class CPU
{
public:
    CPU();

    void run();

    void loadProgram(unsigned char *program, int size);

    void executeCycle();
    void executeInstruction();

    void notImplementedInstruction();

private:

    //General purpose registers
    register8 A;
    register8 X;
    register8 Y;

    //Program counter
    register16 pc;

    //Stack Pointer
    register8 sp;

    //Status register     --->   [ N | V |   | B | D | I | Z | C ]
    register8 P;          //Bits:  7   6   5   4   3   2   1   0

    //RAM (64Kb)
    register8 memory[0x10000];

    bool HLT;

    CPUInstruction currentInstruction;

    int totalCycles;
    
    CPUInstruction decodeInstruction();

    void printCPUState();
    
    //Utilities
    int joinBytes(uint8_t msB, uint8_t lsB);
    uint8_t lowByte(int data);
    uint8_t highByte(int data);

    //Address modes
    int calculateZeroPageAddress(uint8_t ADL);

    //Flags
    void set_N_Flag(bool set);
    void set_Z_Flag(bool set);
    void set_C_Flag(bool set);

    //Stack
    void pushToStack(int data);
    void pushToStack(uint8_t HByte, uint8_t LByte);
};

#endif // CPU_H
