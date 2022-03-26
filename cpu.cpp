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
    P = 0x24;       //EL bit que sobra a 1 y el Bit interrupciones disabled a 1. //En teoría es 0x34, pero el emulador de prueba lo inicializa así y de este modo me cuadra el log.

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
    //The instruction is decoded only on its first cycle
    if(!currentInstruction.IsDecoded)
    {
        currentInstruction = decodeInstruction();
        printCPUState();
    }

    //We simulate one cycle
    currentInstruction.Cycles--;
    totalCycles++;

    //Once we have simulated all the instruction cycles, we execute the whole instruction in the last cycle
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
        //Bit 4 doesn't exist on the status register, only when it is pushed to the stack.  Ref: https://www.nesdev.org/wiki/Status_flags
        uint8_t statusRegister = (P | 0b00010000);          //Bit4 = 1
        pushToStack_1Byte(statusRegister);
        break;
    }
    case ORA_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        uint8_t value = memory[address];

        uint8_t result = A | value;
        loadRegister(&A,result);

        break;
    }
    case ORA_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t value = memory[address];

        uint8_t result = A | value;
        loadRegister(&A,result);
        break;
    }
    case ORA_I:
    {
        uint8_t value = currentInstruction.Data1;

        uint8_t result = A | value;
        loadRegister(&A,result);
        break;
    }
    case ASL_A:
    {
        set_C_Flag(A & 0b10000000);
        uint8_t shiftedValue = A << 1;

        loadRegister(&A,shiftedValue);
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

        pc +=2;

        pushToStack_2Bytes(pc);

        pc = joinBytes(ADH,ADL);
        break;
    }
    case AND_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        uint8_t value = memory[address];

        uint8_t result = A & value;
        loadRegister(&A,result);
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
      case AND_ZP:
      {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t value = memory[address];

        uint8_t result = A & value;
        loadRegister(&A,result);
        break;
      }
    case PLP:
    {
        //Bit 4 doesn't exist on the status register, only when it is pushed to the stack. Bit 5 is always high.  Ref: https://www.nesdev.org/wiki/Status_flags
        uint8_t statusRegister = (pullFromStack_1Byte() & 0b11101111);  //Bit4 = 0
        statusRegister |= 0b00100000;                                   //Bit5 = 1
        P = statusRegister;
        break;
    }
    case AND_I:
    {
        uint8_t value = currentInstruction.Data1;

        uint8_t result = A & value;
        loadRegister(&A,result);
        break;
    }
    case ROL_A:
    {
        uint8_t shiftedValue = (A << 1) | C_FlagSet();
        set_C_Flag(A & 0b10000000);

        loadRegister(&A,shiftedValue);
        break;
    }
    case BMI:
    {
        uint8_t offset = currentInstruction.Data1;

        if(N_FlagSet())
        {
            int newAddress = calculateRelativeAddress(offset);

            pc = newAddress;
        }
        break;
    }
    case SEC:
    {
        set_C_Flag(true);
        break;
    }
    case RTI:
    {
        //Bit 4 doesn't exist on the status register, only when it is pushed to the stack. Bit 5 is always high.  Ref: https://www.nesdev.org/wiki/Status_flags
        uint8_t statusRegister = (pullFromStack_1Byte() & 0b11101111);  //Bit4 = 0
        statusRegister |= 0b00100000;                                   //Bit5 = 1
        P = statusRegister;
        pc = pullFromStack_2Bytes();

        break;
    }
    case EOR_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        uint8_t value = memory[address];

        uint8_t result = A ^ value;
        loadRegister(&A,result);
        break;
    }
    case EOR_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t value = memory[address];

        uint8_t result = A ^ value;
        loadRegister(&A,result);
        break;
    }
    case PHA:
    {
        pushToStack_1Byte(A);
        break;
    }
    case EOR_I:
    {
        uint8_t value = currentInstruction.Data1;

        uint8_t result = A ^ value;
        loadRegister(&A,result);
        break;
    }
    case LSR_A:
    {
        set_C_Flag(A & 0b00000001);
        uint8_t shiftedValue = (A >> 1);
        loadRegister(&A,shiftedValue);
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

        pc++;

        break;
    }
    case ADC_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        uint8_t operand = memory[address];

        uint16_t result = A + operand + (uint8_t)C_FlagSet();

        set_C_Flag(result > 0xFF);
        set_V_Flag(operationHasOverflow(A,operand,result));

        loadRegister(&A,result);     //loadRegister function already considers N and Z flags.
        break;
    }
    case ADC_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t operand = memory[address];

        uint16_t result = A + operand + (uint8_t)C_FlagSet();

        set_C_Flag(result > 0xFF);
        set_V_Flag(operationHasOverflow(A,operand,result));

        loadRegister(&A,result);     //loadRegister function already considers N and Z flags.
        break;
    }
    case PLA:
    {
        uint8_t value = pullFromStack_1Byte();
        loadRegister(&A,value);
        break;
    }
    case ADC_I:
    {
        uint8_t operand = currentInstruction.Data1;
        uint16_t result = A + operand + (uint8_t)C_FlagSet();

        set_C_Flag(result > 0xFF);
        set_V_Flag(operationHasOverflow(A,operand,result));

        loadRegister(&A,result);     //loadRegister function already considers N and Z flags.
        break;
    }
    case ROR_A:
    {
        uint8_t C = C_FlagSet();
        uint8_t shiftedValue = (A >> 1) | (C << 7);
        set_C_Flag(A & 0b00000001);

        loadRegister(&A,shiftedValue);
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
        break;
    }
    case STA_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        storeValueInMemory(A,address);
        break;
    }
    case STY_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        storeValueInMemory(Y,address);
        break;
    }
    case STA_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        storeValueInMemory(A,address);
        break;
    }
    case STX_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        storeValueInMemory(X,address);
        break;
    }
    case DEY:
    {
        uint8_t result = Y-1;
        loadRegister(&Y,result);
        break;
    }
    case TXA:
    {
        loadRegister(&A,X);
        break;
    }
    case STA_ABS:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        storeValueInMemory(A,ADH,ADL);
        break;
    }
    case STX_ABS:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        storeValueInMemory(X,ADH,ADL);
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
    case TYA:
    {
        loadRegister(&A,Y);
        break;
    }
    case TXS:
    {
        sp = X;
        break;
    }
    case LDY_I:
    {
        uint8_t Oper = currentInstruction.Data1;
        loadRegister(&Y,Oper);
        break;
    }
    case LDA_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);

        loadRegister(&A,memory[address]);
        break;
    }
    case LDX_I:
    {
        uint8_t Oper = currentInstruction.Data1;
        loadRegister(&X,Oper);
        break;
    }
    case LDY_ZP:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = calculateZeroPageAddress(ADL);

        loadRegister(&Y,memory[address]);
        break;
    }
    case LDA_ZP:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = calculateZeroPageAddress(ADL);

        loadRegister(&A,memory[address]);
        break;
    }
    case LDX_ZP:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = calculateZeroPageAddress(ADL);

        loadRegister(&X,memory[address]);
        break;
    }
    case TAY:
    {
        loadRegister(&Y,A);
        break;
    }
    case LDA_I:
    {
        uint8_t Oper = currentInstruction.Data1;
        loadRegister(&A,Oper);
        break;
    }
    case TAX:
    {
        loadRegister(&X,A);
        break;
    }
    case LDA_ABS:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        uint8_t value = memory[joinBytes(ADH,ADL)];

        loadRegister(&A,value);
        break;
    }
    case LDX_ABS:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint8_t ADH = currentInstruction.Data2;

        uint8_t value = memory[joinBytes(ADH,ADL)];

        loadRegister(&X,value);
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
    case CLV:
    {
        set_V_Flag(false);
        break;
    }
    case TSX:
    {
        loadRegister(&X,sp);
        break;
    }
    case CPY_I:
    {
        uint8_t value = currentInstruction.Data1;
        uint8_t result = Y - value;

        set_Z_Flag(result == 0);
        set_N_Flag(result >> 7);
        set_C_Flag(Y >= value);

        break;
    }
    case CMP_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        uint8_t value = memory[address];

        uint8_t result = A - value;
        set_Z_Flag(result == 0);
        set_N_Flag(result >> 7);
        set_C_Flag(value <= A);
        break;
    }
    case CMP_ZP:
    {
        int address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t value = memory[address];

        uint8_t result = A - value;
        set_Z_Flag(result == 0);
        set_N_Flag(result >> 7);
        set_C_Flag(value <= A);
        break;
    }
    case INY:
    {
        uint8_t result = Y+1;
        loadRegister(&Y,result);
        break;
    }
    case CMP_I:
    {
        uint8_t value = currentInstruction.Data1;
        uint8_t result = A - value;

        set_Z_Flag(result == 0);
        set_N_Flag(result >> 7);
        set_C_Flag(value <= A);
        break;
    }
    case DEX:
    {
        uint8_t result = X-1;
        loadRegister(&X,result);
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
    case CLD:
    {
        set_D_Flag(false);
        break;
    }
    case CPX_I:
    {
        uint8_t value = currentInstruction.Data1;
        uint8_t result = X - value;

        set_Z_Flag(result == 0);
        set_N_Flag(result >> 7);
        set_C_Flag(X >= value);

        break;
    }
    case SBC_IX:
    {
        uint8_t ADL = currentInstruction.Data1;
        uint16_t address = indirectIndexedAddress(ADL,&X);
        uint8_t operand = ~memory[address];

        uint16_t result = A + operand + (uint8_t)C_FlagSet();       //Parece que funciona bien sin invertir Carry Flag??
        set_C_Flag(result > 0xFF);
        set_V_Flag(operationHasOverflow(A,operand,result));

        loadRegister(&A,result);
        break;
    }
    case SBC_ZP:
    {
        uint16_t address = calculateZeroPageAddress(currentInstruction.Data1);
        uint8_t operand = ~memory[address];

        uint16_t result = A + operand + (uint8_t)C_FlagSet();       //Parece que funciona bien sin invertir Carry Flag??
        set_C_Flag(result > 0xFF);
        set_V_Flag(operationHasOverflow(A,operand,result));

        loadRegister(&A,result);
        break;
    }
    case INX:
    {
        uint8_t result = X+1;
        loadRegister(&X,result);
        break;
    }
    case SBC_I:
    {
        uint8_t operand = ~currentInstruction.Data1;

        uint16_t result = A + operand + (uint8_t)C_FlagSet();       //Parece que funciona bien sin invertir Carry Flag??
        set_C_Flag(result > 0xFF);
        set_V_Flag(operationHasOverflow(A,operand,result));

        loadRegister(&A,result);
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
    printf("INSTRUCTION NOT IMPLEMENTED: %02X\n\n", memory[pc]);
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
    case ORA_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case ORA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case ORA_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case ASL_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case BPL:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(N_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc+2,newAddress))
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
    case AND_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case BIT_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case AND_ZP:
    {
      return CPUInstruction(opCode,data1,3,false);
    }
    case PLP:
    {
        return CPUInstruction(opCode,4,false);
    }
    case AND_I:
    {
        return  CPUInstruction(opCode,data1,2,false);
    }
    case ROL_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case BMI:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!N_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case SEC:
    {
        return  CPUInstruction(opCode,2,false);
    }
    case RTI:
    {
        return CPUInstruction(opCode,6,true);
    }
    case EOR_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case EOR_ZP:
    {
      return CPUInstruction(opCode,data1,3,false);
    }
    case PHA:
    {
        return CPUInstruction(opCode,3,false);
    }
    case EOR_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case LSR_A:
    {
        return CPUInstruction(opCode,2,false);
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

        if(samePageAddresses(pc+2,newAddress))
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
    case ADC_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case ADC_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case PLA:
    {
        return CPUInstruction(opCode,4,false);
    }
    case ADC_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case ROR_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case BVS:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!V_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc+2,newAddress))
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
    case STA_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case STY_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case STA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case STX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case DEY:
    {
        return CPUInstruction(opCode,2,false);
    }
    case TXA:
    {
        return CPUInstruction(opCode,2,false);
    }
    case STA_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case STX_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case BCC:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(C_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case TYA:
    {
        return CPUInstruction(opCode,2,false);
    }
    case TXS:
    {
        return CPUInstruction(opCode,2,false);
    }
    case LDY_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case LDA_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case LDX_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case LDY_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case LDA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case LDX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case TAY:
    {
        return CPUInstruction(opCode,2,false);
    }
    case LDA_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case TAX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case LDA_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case LDX_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case BCS:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!C_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case CLV:
    {
        return CPUInstruction(opCode,2,false);
    }
    case TSX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case CPY_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case CMP_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case CMP_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case INY:
    {
        return CPUInstruction(opCode,2,false);
    }
    case CMP_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case DEX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case BNE:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(Z_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        int newAddress = calculateRelativeAddress(data1);

        if(samePageAddresses(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case CLD:
    {
        return CPUInstruction(opCode,2,false);
    }
    case CPX_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case SBC_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case SBC_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case INX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case SBC_I:
    {
        return CPUInstruction(opCode,data1,2,false);
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

        if(samePageAddresses(pc+2,newAddress))
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

uint16_t CPU::indirectIndexedAddress(uint8_t zp_ADL, register8 *index)
{
    uint16_t zp_Address1 = calculateZeroPageAddress(zp_ADL + *index);
    uint16_t zp_Address2 = calculateZeroPageAddress(zp_ADL + *index + 1);
    uint8_t ADL = memory[zp_Address1];
    uint8_t ADH = memory[zp_Address2];

    return joinBytes(ADH,ADL);
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

bool CPU::operationHasOverflow(uint8_t a, uint8_t b, uint8_t result)
{
    uint8_t signA = (a >> 7);
    uint8_t signOperand = (b >> 7);
    uint8_t signResult = (result >> 7);

    if (signA != signOperand)
    {
        return false;
    }
    else if (signResult != signA)
    {
        return true;
    }
    else return false;
}

void CPU::pushToStack_2Bytes(int data)
{
    pushToStack_2Bytes(highByte(data),lowByte(data));
}

void CPU::pushToStack_2Bytes(uint8_t HByte, uint8_t LByte)
                            //Y ADEMÁS. Siempre que estoy utilizando ints con signo para acceder a memory[] igual la estoy liando porque si es dirección de memoria alta saldrá  memory[-46].
{
    pushToStack_1Byte(HByte);
    pushToStack_1Byte(LByte);
}

void CPU::pushToStack_1Byte(uint8_t data)
{
    memory[joinBytes(0x01,sp)] = data;

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
    return memory[joinBytes(0x01,sp)];
}

void CPU::loadRegister(register8 *Reg, uint8_t value)
{
    *Reg = value;

    set_Z_Flag(*Reg == 0);
    set_N_Flag(*Reg >> 7);
}

void CPU::storeValueInMemory(uint8_t value, uint8_t ADH, uint8_t ADL)
{
    storeValueInMemory(value,joinBytes(ADH,ADL));
}

void CPU::storeValueInMemory(uint8_t value, uint16_t address)
{
    memory[address] = value;
}
