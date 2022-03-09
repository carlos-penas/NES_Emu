#include "cpu.h"
#include <cstring>
#include "cpuOpCodes.h"
#include "stdio.h"

CPU::CPU()
{
    A = 0;
    X = 0;
    Y = 0;

    pc = 0xC000;
    sp = 0xFD;      //Empieza en FB, habrá que leerse la docu.
    P = 0x24;       //EL bit que sobra a 1 y el Bit interrupciones disabled a 1.

    HLT = false;

    currentInstruction = CPUInstruction();
    totalCycles = 7;
}

void CPU::run()
{
    while(!HLT)
    {
        executeCycle();
    }
    printf("\nHalting the system...\n");
}

void CPU::loadProgram(unsigned char *program, int size)
{
    memcpy(&memory[0xBFF0],program,size);
//    printf("Primera instrucción: %02x\n", memory[0xC000]);
//    printf("Luego: %02x\n", memory[0xC001]);
//    printf("Luego: %02x\n", memory[0xC002]);
//    printf("Últimas: %02x\n", memory[0xFFFA]);
//    printf("Últimas: %02x\n", memory[0xFFFB]);
//    printf("Últimas: %02x\n", memory[0xFFFC]);
//    printf("Últimas: %02x\n", memory[0xFFFD]);
//    printf("Últimas: %02x\n", memory[0xFFFE]);
    //    printf("Últimas: %02x\n", memory[0xFFFF]);
}

void CPU::executeCycle()
{
    //The instruction is decoded only on its first cycle;
    if(!currentInstruction.IsDecoded)
    {
        currentInstruction = decodeInstruction();
        printCPUState();
    }

    //We simulate one cycle
    currentInstruction.Cycles--;
    totalCycles++;

    //Once we have simulated all the instruction cycles, we execute the instruction
    if(currentInstruction.Cycles == 0)
    {
        executeInstruction();
        currentInstruction.IsDecoded = false;

        //The pc is only updated if the instruction is not a branch instruction
        if(!currentInstruction.jumpInstruction)
        {
            pc+=currentInstruction.Bytes;
        }

    }
}

void CPU::executeInstruction()
{
    uint8_t opCode = currentInstruction.OpCode;

    switch (opCode){
    case JSR:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        pushToStack(pc);

        pc = joinBytes(ADH,ADL);
        break;
    }
    case SEC:
    {
        set_C_Flag(true);
        break;
    }
    case JMP_ABS:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        pc = joinBytes(ADH,ADL);

        break;
    }
    case STX_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        memory[address] = X;

        break;
    }
    case LDX_I:
    {
        uint8_t Oper = currentInstruction.Data1;
        X = Oper;

        set_Z_Flag(X == 0);
        set_N_Flag(X >> 7);

        break;
    }
    case NOP:
    {
        //No operation
        break;
    }
    default:
        notImplementedInstruction();
    }
}

void CPU::notImplementedInstruction()
{
    printf("INSTRUCTION NOT IMPLEMENTED: %02x\n\n", memory[pc]);
    HLT = true;
}

CPUInstruction CPU::decodeInstruction()
{
    uint8_t opCode = memory[pc];
    uint8_t data1;
    uint8_t data2;

    data1 = memory[pc+1];
    data2 = memory[pc+2];

    switch (opCode){
    case JSR:
    {
        return CPUInstruction(opCode,data1,data2,6,true);
    }
    case SEC:
    {
        return  CPUInstruction(opCode,2,false);
    }
    case JMP_ABS:
    {
        return CPUInstruction(opCode,data1,data2,3,true);
    }
    case STX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case LDX_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case BCS:
    {

    }
    case NOP:
    {
        return CPUInstruction(opCode,2,false);
    }
    default:
        notImplementedInstruction();
        return CPUInstruction();
    }
}

void CPU::printCPUState()
{
    if (currentInstruction.Bytes == 1)
    {
        printf("%04X  %02X                          A:%02X X:%02X Y:%02X P:%02X SP:%02X  CYC:%d\n", pc, currentInstruction.OpCode, A, X, Y, P, sp, totalCycles);
    }
    else if(currentInstruction.Bytes == 2)
    {
        printf("%04X  %02X %02X                       A:%02X X:%02X Y:%02X P:%02X SP:%02X  CYC:%d\n", pc, currentInstruction.OpCode, currentInstruction.Data1, A, X, Y, P, sp, totalCycles);
    }
    else
    {
        printf("%04X  %02X %02X %02X                    A:%02X X:%02X Y:%02X P:%02X SP:%02X  CYC:%d\n", pc, currentInstruction.OpCode, currentInstruction.Data1, currentInstruction.Data2, A, X, Y, P, sp, totalCycles);
    }
}

int CPU::joinBytes(uint8_t msB, uint8_t lsB)
{
    return (msB << 8) | lsB;
}

uint8_t CPU::lowByte(int data)
{
    return (uint8_t) (data & 0x0F);
}

uint8_t CPU::highByte(int data)
{
    return  (uint8_t) (data >> 4);
}

int CPU::calculateZeroPageAddress(uint8_t ADL)
{
    return joinBytes(0x00,ADL);
}

void CPU::set_N_Flag(bool set)
{
    if (set)
        P |= 0b10000000;
    else
        P &= 0b01111111;
}

void CPU::set_Z_Flag(bool set)
{
    if(set)
        P |= 0b00000010;
    else
        P &= 0b11111101;
}

void CPU::set_C_Flag(bool set)
{
    if(set)
        P |= 0b00000001;
    else
        P &= 0b11111110;
}

void CPU::pushToStack(int data)
{
    pushToStack(highByte(data),lowByte(data));
}

void CPU::pushToStack(uint8_t HByte, uint8_t LByte)
{
    memory[sp]     = HByte;
    memory[sp - 1] = LByte;

    sp -= 2;
}
