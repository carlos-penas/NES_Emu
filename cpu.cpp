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
    sp = 0xFD;      //Empieza en FD, habrá que leerse la docu.
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
    case PHP:
    {
        pushToStack_1Byte(P);
        break;
    }
    case BPL:
    {
        uint8_t offset = currentInstruction.Data1;

        if(!N_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case CLC:
    {
        set_C_Flag(false);
        break;
    }
    case JSR:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        pushToStack_2Bytes(pc);

        pc = joinBytes(ADH,ADL);
        break;
    }
    case BIT_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t value = memory[address];

        set_N_Flag(value >> 7);
        set_V_Flag((value & 0b01000000) >> 6);

        value &= A;

        set_Z_Flag(value == 0);

        break;
    }
    case AND_I:
    {
        uint8_t value = currentInstruction.Data1;
        uint8_t result = A & value;

        loadRegister(&A,result);
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
    case BVC:
    {
        uint8_t offset = currentInstruction.Data1;

        if(!V_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case RTS:
    {
        pc = pullFromStack_2Bytes();
        pc +=3; //3 bytes to advance the previous JSR instruction
        break;
    }
    case PLA:
    {
        uint8_t value = pullFromStack_1Byte();
        loadRegister(&A,value);
        break;
    }
    case BVS:
    {
        uint8_t offset = currentInstruction.Data1;

        if(V_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case SEI:
    {
        set_I_Flag(true);
        return;
    }
    case STA_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        memory[address] = A;

        break;
    }
    case STX_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        memory[address] = X;

        break;
    }
    case BCC:
    {
        uint8_t offset = currentInstruction.Data1;

        if(!C_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case LDX_I:
    {
        uint8_t Oper = currentInstruction.Data1;
        loadRegister(&X,Oper);
        break;
    }
    case LDA_I:
    {
        uint8_t Oper = currentInstruction.Data1;
        loadRegister(&A,Oper);
        break;
    }
    case BCS:
    {
        uint8_t offset = currentInstruction.Data1;

        if(C_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case BNE:
    {
        uint8_t offset = currentInstruction.Data1;

        if(!Z_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case NOP:
    {
        //No operation
        break;
    }
    case BEQ:
    {
        uint8_t offset = currentInstruction.Data1;

        if(Z_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case SED:
    {
        set_D_Flag(true);
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
    case PHP:
    {
        return CPUInstruction(opCode,3,false);
    }
    case BPL:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(N_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case CLC:
    {
        return CPUInstruction(opCode,2,false);
    }
    case JSR:
    {
        return CPUInstruction(opCode,data1,data2,6,true);
    }
    case BIT_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case AND_I:
    {
        return  CPUInstruction(opCode,data1,2,false);
    }
    case SEC:
    {
        return  CPUInstruction(opCode,2,false);
    }
    case JMP_ABS:
    {
        return CPUInstruction(opCode,data1,data2,3,true);
    }
    case BVC:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(V_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case RTS:
    {
        return CPUInstruction(opCode,6,true);
    }
    case PLA:
    {
        return CPUInstruction(opCode,4,false);
    }
    case BVS:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!V_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case SEI:
    {
        return  CPUInstruction(opCode,2,false);
    }
    case STA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case STX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case BCC:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(C_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case LDX_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case LDA_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case BCS:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!C_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case BNE:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(Z_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case NOP:
    {
        return CPUInstruction(opCode,2,false);
    }
    case BEQ:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!Z_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case SED:
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
    return (uint8_t) (data & 0xFF);
}

uint8_t CPU::highByte(int data)
{
    return  (uint8_t) (data >> 8);
}

int CPU::calculateZeroPageAddress(uint8_t ADL)
{
    return joinBytes(0x00,ADL);
}

int CPU::calculateRelativeAddress(uint8_t Offset)
{
    int8_t signedOffset = (int8_t) Offset;

    return pc + signedOffset;
}

bool CPU::samePageAddresses(int add1, int add2)
{
    return (highByte(add1) == highByte(add2));
}

void CPU::set_N_Flag(bool set)
{
    if (set)
        P |= 0b10000000;
    else
        P &= 0b01111111;
}

void CPU::set_V_Flag(bool set)
{
    if(set)
        P |= 0b01000000;
    else
        P &= 0b10111111;
}

void CPU::set_D_Flag(bool set)
{
    if(set)
        P |= 0b00001000;
    else
        P &= 0b11110111;
}

void CPU::set_I_Flag(bool set)
{
    if(set)
        P |= 0b00000100;
    else
        P &= 0b11111011;
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

bool CPU::N_FlagSet()
{
    return (P & 0b10000000);
}

bool CPU::V_FlagSet()
{
    return (P & 0b01000000);
}

bool CPU::D_FlagSet()
{
    return (P & 0b00001000);
}

bool CPU::I_FlagSet()
{
    return (P & 0b00000100);
}

bool CPU::Z_FlagSet()
{
    return (P & 0b00000010);
}

bool CPU::C_FlagSet()
{
    return (P & 0b00000001);
}

void CPU::pushToStack_2Bytes(int data)
{
    pushToStack_2Bytes(highByte(data),lowByte(data));
}

void CPU::pushToStack_2Bytes(uint8_t HByte, uint8_t LByte) //CUIDAO: El stack pointer es de 8 bits. Y lo estoy metiendo a una dirección de 16. Igual hay que concatenarle algo.
                            //Y ADEMÁS. Siempre que estoy utilizando ints con signo para acceder a memory[] igual la estoy liando porque si es dirección de memoria alta saldrá  memory[-46].
{
    pushToStack_1Byte(HByte);
    pushToStack_1Byte(LByte);
}

void CPU::pushToStack_1Byte(uint8_t data)
{
    memory[sp] = data;

    sp--;
}

int CPU::pullFromStack_2Bytes()
{
    uint8_t LByte = pullFromStack_1Byte();
    uint8_t HByte = pullFromStack_1Byte();

    return joinBytes(HByte,LByte);

}

uint8_t CPU::pullFromStack_1Byte()
{
    sp +=1;
    return memory[sp];
}

void CPU::loadRegister(register8 *Reg, uint8_t value)
{
    *Reg = value;

    set_Z_Flag(*Reg == 0);
    set_N_Flag(*Reg >> 7);
}
