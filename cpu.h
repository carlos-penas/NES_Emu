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

    //Addressing modes
    int calculateZeroPageAddress(uint8_t ADL);
    int calculateRelativeAddress(uint8_t Offset);

    bool samePageAddresses(int add1, int add2);

    //Flags
    void set_N_Flag(bool set);
    void set_V_Flag(bool set);
    void set_Z_Flag(bool set);
    void set_C_Flag(bool set);

    bool N_FlagSet();
    bool V_FlagSet();
    bool Z_FlagSet();
    bool C_FlagSet();


    //Stack
    void pushToStack(int data);
    void pushToStack(uint8_t HByte, uint8_t LByte);

    int pullFromStack_2Bytes();
    uint8_t pullFromStack_1Byte();

    //Registers
    void loadRegister(register8 *reg, uint8_t value);
};

#endif // CPU_H
