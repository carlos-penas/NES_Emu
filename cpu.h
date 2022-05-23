#ifndef CPU_H
#define CPU_H
#include <cstdint>
#include <cpuinstruction.h>

typedef uint8_t register8;
typedef uint16_t register16;
typedef uint8_t Byte;
typedef uint16_t Address;

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
    Address joinBytes(Byte msB, Byte lsB);
    Byte lowByte(Address data);
    Byte highByte(Address data);

    //Addressing modes
    Address zeroPageAddress(Byte ADL);
    Address relativeAddress(Byte Offset);
    Address zeroPageIndexedAddress(Byte ADL, register8 *index);
    Address indirectAddress(Byte IAH, Byte ADL);
    Address absoluteIndexedAddress(Byte ADH, Byte ADL, register8 *index);
    Address indexedIndirectAddress(Byte zp_ADL, register8 *index);
    Address indirectIndexedAddress(Byte zp_ADL, register8 *index);

    bool samePageAddresses(Address add1, Address add2);

    //Flags
    void set_N_Flag(bool set);
    void set_V_Flag(bool set);
    void set_D_Flag(bool set);
    void set_I_Flag(bool set);
    void set_Z_Flag(bool set);
    void set_C_Flag(bool set);

    bool N_FlagSet();
    bool V_FlagSet();
    bool D_FlagSet();
    bool I_FlagSet();
    bool Z_FlagSet();
    bool C_FlagSet();

    bool operationHasOverflow(Byte a, Byte b, Byte result);

    //Stack
    void pushToStack_2Bytes(Address data);
    void pushToStack_2Bytes(Byte HByte, Byte LByte);
    void pushToStack_1Byte(Byte data);
    Address pullFromStack_2Bytes();
    Byte pullFromStack_1Byte();

    //Registers
    void loadRegister(register8 *reg, Byte value);

    //Memory
    void storeValueInMemory(Byte value, Byte ADH, Byte ADL, bool checkFlags);
    void storeValueInMemory(Byte value, Address address, bool checkFlags);

    //Official Instructions
    void ADC(Byte operand);    void AND(Byte value);      void ASL(Address address); void BCC();                void BCS();                void BEQ();
    void BIT(Byte value);      void BMI();                void BNE();                void BPL();                void BRK();                void BVC();
    void BVS();                void CLC();                void CLD();                void CLI();                void CLV();                void CMP(Byte value);
    void CPX(Byte value);      void CPY(Byte value);      void DEC(Address address); void DEX();                void DEY();                void EOR(Byte value);
    void INC(Address address); void INX();                void INY();                void JMP(Address address); void JSR();                void LDA(Byte value);
    void LDX(Byte value);      void LDY(Byte value);      void LSR(Address address); void NOP();                void ORA(Byte value);      void PHA();
    void PHP();                void PLA();                void PLP();                void ROL(Address address); void ROR(Address address); void RTI();
    void RTS();                void SBC(Byte operand);    void SEC();                void SED();                void SEI();                void STA(Address address);
    void STX(Address address); void STY(Address address); void TAX();                void TAY();                void TSX();                void TXA();
    void TXS();                void TYA();

    //Unofficial/Illegal Instructions
    void DCP(Address address);
    void DOP();
    void ISC(Address address);
    void LAX(Address address);
    void RLA(Address address);
    void RRA(Address address);
    void SAX(Address address);
    void SLO(Address address);
    void SRE(Address address);
    void TOP();
};

#endif // CPU_H
