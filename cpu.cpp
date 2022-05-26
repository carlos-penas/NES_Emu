#include "cpu.h"
#include "cpuOpCodes.h"
#include "utils.h"
#include "stdio.h"
#include <cstring>

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
    printf("NES TEST RESULTS: %02X %02X\n", memoryRead(0x02), memoryRead(0x03));
//    printf("Instr_timing RESULTS:\n");
//    printf("State: %02X\n", memory[0x6000]);
//    printf("%s", memory[0x6004]);
}

void CPU::loadProgram(unsigned char *program, int size)
{
    memcpy(&Cartridge[0xBFE0-size],program,size);

    printf("Primera instrucción: %02x\n", memoryRead(0xC000));
    printf("Luego: %02x\n", memoryRead(0xC001));
    printf("Luego: %02x\n", memoryRead(0xC002));
    printf("Últimas: %02x\n", memoryRead(0xFFFA));
    printf("Últimas: %02x\n", memoryRead(0xFFFB));
    printf("Últimas: %02x\n", memoryRead(0xFFFC));
    printf("Últimas: %02x\n", memoryRead(0xFFFD));
    printf("Últimas: %02x\n", memoryRead(0xFFFE));
    printf("Últimas: %02x\n", memoryRead(0xFFFF));
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
    Byte opCode = currentInstruction.OpCode;

    switch (opCode){
    case opCodes::ORA_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        ORA(memoryRead(address));
        break;
    }
    case 0x03:
    {
        //Illegal: SLO_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        SLO(address);
        break;
    }
    case 0x04:
    {
        //Illegal: DOP
        DOP();
        break;
    }
    case opCodes::ORA_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        ORA(memoryRead(address));
        break;
    }
    case opCodes::ASL_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        ASL(address);
        break;
    }
    case 0x07:
    {
        //Illegal SLO_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        SLO(address);
        break;
    }
    case opCodes::PHP:
    {
        PHP();
        break;
    }
    case opCodes::ORA_I:
    {
        ORA(currentInstruction.Data1);
        break;
    }
    case opCodes::ASL_A:
    {
        set_C_Flag(A & 0b10000000);
        Byte shiftedValue = A << 1;
        loadRegister(&A,shiftedValue);
        break;
    }
    case 0x0C:
    {
        //Illegal: TOP
        TOP();
        break;
    }
    case opCodes::ORA_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        ORA(memoryRead(address));
        break;
    }
    case opCodes::ASL_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        ASL(address);
        break;
    }
    case 0x0F:
    {
        //Illegal: SLO_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        SLO(address);
        break;
    }
    case opCodes::BPL:
    {
        BPL();
        break;
    }
    case opCodes::ORA_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        ORA(memoryRead(address));
        break;
    }
    case 0x13:
    {
        //Illegal: SLO_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        SLO(address);
        break;
    }
    case 0x14:
    {
        //Illegal: DOP
        DOP();
        break;
    }
    case opCodes::ORA_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        ORA(memoryRead(address));
        break;
    }
    case opCodes::ASL_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        ASL(address);
        break;
    }
    case 0x17:
    {
        //Illegal: SLO_ZP_X
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        SLO(address);
        break;
    }
    case opCodes::CLC:
    {
        CLC();
        break;
    }
    case opCodes::ORA_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        ORA(memoryRead(address));
        break;
    }
    case 0x1A:
    {
        //Illegal: NOP
        NOP();
        break;
    }
    case 0x1B:
    {
        //Illegal: SLO_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        SLO(address);
        break;
    }
    case 0x1C:
    {
        //Illegal: TOP_ABS_X
        TOP();
        break;
    }
    case opCodes::ORA_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        ORA(memoryRead(address));
        break;
    }
    case opCodes::ASL_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        ASL(address);
        break;
    }
    case 0x1F:
    {
        //Illegal: SLO_ABS_X
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        SLO(address);
        break;
    }
    case opCodes::JSR:
    {
        JSR();
        break;
    }
    case opCodes::AND_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        AND(memoryRead(address));
        break;
    }
    case 0x23:
    {
        //Illegal: RLA_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        RLA(address);
        break;
    }
    case opCodes::BIT_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        BIT(memoryRead(address));
        break;
    }
    case opCodes::AND_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        AND(memoryRead(address));
        break;
    }
    case opCodes::ROL_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        ROL(address);
        break;
    }
    case 0x27:
    {
        //Illegal: RLA_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        RLA(address);
        break;
    }
    case opCodes::PLP:
    {
        PLP();
        break;
    }
    case opCodes::AND_I:
    {
        AND(currentInstruction.Data1);
        break;
    }
    case opCodes::ROL_A:
    {
        Byte shiftedValue = (A << 1) | C_FlagSet();
        set_C_Flag(A & 0b10000000);
        loadRegister(&A,shiftedValue);
        break;
    }
    case opCodes::BIT_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        BIT(memoryRead(address));
        break;
    }
    case opCodes::AND_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address =  Utils::joinBytes(ADH,ADL);
        AND(memoryRead(address));
        break;
    }
    case opCodes::ROL_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        ROL(address);
        break;
    }
    case 0x2F:
    {
        //Illegal: RLA_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        RLA(address);
        break;
    }
    case opCodes::BMI:
    {
        BMI();
        break;
    }
    case opCodes::AND_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        AND(memoryRead(address));
        break;
    }
    case 0x33:
    {
        //Illegal: RLA_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        RLA(address);
        break;
    }
    case 0x34:
    {
        //Illegal: DOP_ZP_X
        DOP();
        break;
    }
    case opCodes::AND_ZP_X:
    {
        Byte ADL = currentInstruction.Data1;
        Address address = zeroPageIndexedAddress(ADL,&X);
        Byte value = memoryRead(address);

        Byte result = A & value;
        loadRegister(&A,result);
        break;
    }
    case opCodes::ROL_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        ROL(address);
        break;
    }
    case 0x37:
    {
        //Illegal: RLA_ZP_X
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        RLA(address);
        break;
    }
    case opCodes::SEC:
    {
        SEC();
        break;
    }
    case opCodes::AND_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        AND(memoryRead(address));
        break;
    }
    case 0x3A:
    {
        //Illegal: NOP
        NOP();
        break;
    }
    case 0x3B:
    {
        //Illegal: RLA_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        RLA(address);
        break;
    }
    case 0x3C:
    {
        //Illegal: TOP_ABS_X
        TOP();
        break;
    }
    case opCodes::AND_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        AND(memoryRead(address));
        break;
    }
    case opCodes::ROL_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        ROL(address);
        break;
    }
    case 0x3F:
    {
        //Illegal: RLA_ABS_X
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        RLA(address);
        break;
    }
    case opCodes::RTI:
    {
        RTI();
        break;
    }
    case opCodes::EOR_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        EOR(memoryRead(address));
        break;
    }
    case 0x43:
    {
        //Illegal: SRE_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        SRE(address);
        break;
    }
    case 0x44:
    {
        //Illegal: DOP_ZP
        DOP();
        break;
    }
    case opCodes::EOR_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        EOR(memoryRead(address));
        break;
    }
    case opCodes::LSR_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        LSR(address);
        break;
    }
    case 0x47:
    {
        //Illegal: SRE_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        SRE(address);
        break;
    }
    case opCodes::PHA:
    {
        PHA();
        break;
    }
    case opCodes::EOR_I:
    {
        EOR(currentInstruction.Data1);
        break;
    }
    case opCodes::LSR_A:
    {
        set_C_Flag(A & 0b00000001);
        Byte shiftedValue = (A >> 1);
        loadRegister(&A,shiftedValue);
        break;
    }
    case opCodes::JMP_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        JMP(Utils::joinBytes(ADH,ADL));
        break;
    }
    case opCodes::EOR_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        EOR(memoryRead(address));
        break;
    }
    case opCodes::LSR_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        LSR(address);
        break;
    }
    case 0x4F:
    {
        //Illegal: SRE_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        SRE(address);
        break;
    }
    case opCodes::BVC:
    {
        BVC();
        break;
    }
    case opCodes::EOR_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        EOR(memoryRead(address));
        break;
    }
    case 0x53:
    {
        //Illegal: SRE_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        SRE(address);
        break;
    }
    case 0x54:
    {
        //Illegal: DOP_ZP_X
        DOP();
        break;
    }
    case opCodes::EOR_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        EOR(memoryRead(address));
        break;
    }
    case opCodes::LSR_ZP_X:
    {
        Byte ADL = currentInstruction.Data1;
        Address address = zeroPageIndexedAddress(ADL,&X);
        LSR(address);
        break;
    }
    case 0x57:
    {
        //Illegal: SRE_ZP_X
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        SRE(address);
        break;
    }
    case opCodes::EOR_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        EOR(memoryRead(address));
        break;
    }
    case 0x5A:
    {
        //Illegal: NOP
        NOP();
        break;
    }
    case 0x5B:
    {
        //Illegal: SRE_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        SRE(address);
        break;
    }
    case 0x5C:
    {
        //Illegal: TOP_ABS_X
        TOP();
        break;
    }
    case opCodes::EOR_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        EOR(memoryRead(address));
        break;
    }
    case opCodes::LSR_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        LSR(address);
        break;
    }
    case 0x5F:
    {
        //Illegal: SRE_ABS_X
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        SRE(address);
        break;
    }
    case opCodes::RTS:
    {
        RTS();
        break;
    }
    case opCodes::ADC_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        ADC(memoryRead(address));
        break;
    }
    case 0x63:
    {
        //Illegal: RRA_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        RRA(address);
        break;
    }
    case 0x64:
    {
        //Illegal: DOP_ZP
        DOP();
        break;
    }
    case opCodes::ADC_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        ADC(memoryRead(address));
        break;
    }
    case opCodes::ROR_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        ROR(address);
        break;
    }
    case 0x67:
    {
        //Illegal: RRA_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        RRA(address);
        break;
    }
    case opCodes::PLA:
    {
        PLA();
        break;
    }
    case opCodes::ADC_I:
    {
        ADC(currentInstruction.Data1);
        break;
    }
    case opCodes::ROR_A:
    {
        Byte C = C_FlagSet();
        Byte shiftedValue = (A >> 1) | (C << 7);
        set_C_Flag(A & 0b00000001);
        loadRegister(&A,shiftedValue);
        break;
    }
    case opCodes::JMP_I:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        JMP(indirectAddress(ADH,ADL));
        break;
    }
    case opCodes::ADC_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        ADC(memoryRead(address));
        break;
    }
    case opCodes::ROR_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        ROR(address);
        break;
    }
    case 0x6F:
    {
        //Illegal: RRA_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        RRA(address);
        break;
    }
    case opCodes::BVS:
    {
        BVS();
        break;
    }
    case opCodes::ADC_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        ADC(memoryRead(address));
        break;
    }
    case 0x73:
    {
        //Illegal: RRA_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        RRA(address);
        break;
    }
    case 0x74:
    {
        //Illegal: DOP_ZP_X
        DOP();
        break;
    }
    case 0x7C:
    {
        //Illegal: TOP_ABS_X
        TOP();
        break;
    }
    case opCodes::ADC_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        ADC(memoryRead(address));
        break;
    }
    case opCodes::ROR_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        ROR(address);
        break;
    }
    case 0x77:
    {
        //Illegal: RRA_ZP_X
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        RRA(address);
        break;
    }
    case opCodes::SEI:
    {
        SEI();
        break;
    }
    case opCodes::ADC_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        ADC(memoryRead(address));
        break;
    }
    case 0x7A:
    {
        //Illegal: NOP
        NOP();
        break;
    }
    case 0x7B:
    {
        //Illegal: RRA_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        RRA(address);
        break;
    }
    case opCodes::ADC_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        ADC(memoryRead(address));
        break;
    }
    case opCodes::ROR_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        ROR(address);
        break;
    }
    case 0x7F:
    {
        //Illegal: RRA_ABS_X
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        RRA(address);
        break;
    }
    case 0x80:
    {
        //Illegal: DOP
        DOP();
        break;
    }
    case opCodes::STA_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        STA(address);
        break;
    }
    case 0x83:
    {
        //Illegal: SAX_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        SAX(address);
        break;
    }
    case opCodes::STY_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        STY(address);
        break;
    }
    case opCodes::STA_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        STA(address);
        break;
    }
    case opCodes::STX_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        STX(address);
        break;
    }
    case 0x87:
    {
        //Illegal: SAX_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        SAX(address);
        break;
    }
    case opCodes::DEY:
    {
        DEY();
        break;
    }
    case opCodes::TXA:
    {
        TXA();
        break;
    }
    case opCodes::STY_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        STY(Utils::joinBytes(ADH,ADL));
        break;
    }
    case opCodes::STA_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        STA(Utils::joinBytes(ADH,ADL));
        break;
    }
    case opCodes::STX_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        STX(Utils::joinBytes(ADH,ADL));
        break;
    }
    case 0x8F:
    {
        //Illegal: SAX_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        SAX(address);
        break;
    }
    case opCodes::BCC:
    {
        BCC();
        break;
    }
    case opCodes::STA_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        STA(address);
        break;
    }
    case opCodes::STY_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        STY(address);
        break;
    }
    case opCodes::STA_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        STA(address);
        break;
    }
    case opCodes::STX_ZP_Y:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&Y);
        STX(address);
        break;
    }
    case 0x97:
    {
        //Illegal: SAX_ZP_Y
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&Y);
        SAX(address);
        break;
    }
    case opCodes::TYA:
    {
        TYA();
        break;
    }
    case opCodes::STA_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        STA(address);
        break;
    }
    case opCodes::TXS:
    {
        TXS();
        break;
    }
    case opCodes::STA_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        STA(address);
        break;
    }
    case opCodes::LDY_I:
    {
        LDY(currentInstruction.Data1);
        break;
    }
    case opCodes::LDA_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        LDA(memoryRead(address));
        break;
    }
    case opCodes::LDX_I:
    {
        LDX(currentInstruction.Data1);
        break;
    }
    case 0xA3:
    {
        //Illegal: LAX_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        LAX(address);
        break;
    }
    case opCodes::LDY_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        LDY(memoryRead(address));
        break;
    }
    case opCodes::LDA_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        LDA(memoryRead(address));
        break;
    }
    case opCodes::LDX_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        LDX(memoryRead(address));
        break;
    }
    case 0xA7:
    {
        //Illegal: LAX_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        LAX(address);
        break;
    }
    case opCodes::TAY:
    {
        TAY();
        break;
    }
    case opCodes::LDA_I:
    {
        LDA(currentInstruction.Data1);
        break;
    }
    case opCodes::TAX:
    {
        TAX();
        break;
    }
    case opCodes::LDY_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        LDY(memoryRead(address));
        break;
    }
    case opCodes::LDA_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        LDA(memoryRead(address));
        break;
    }
    case opCodes::LDX_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        LDX(memoryRead(address));
        break;
    }
    case 0xAF:
    {
        //Illegal: LAX_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        LAX(address);
        break;
    }
    case opCodes::BCS:
    {
        BCS();
        break;
    }
    case opCodes::LDA_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        LDA(memoryRead(address));
        break;
    }
    case 0xB3:
    {
        //Illegal: LAX_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        LAX(address);
        break;
    }
    case opCodes::LDY_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        LDY(memoryRead(address));
        break;
    }
    case opCodes::LDA_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        LDA(memoryRead(address));
        break;
    }
    case opCodes::LDX_ZP_Y:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&Y);
        LDX(memoryRead(address));
        break;
    }
    case 0xB7:
    {
        //Illegal: LAX_ZP_Y
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&Y);
        LAX(address);
        break;
    }
    case opCodes::CLV:
    {
        CLV();
        break;
    }
    case opCodes::LDA_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        LDA(memoryRead(address));
        break;
    }
    case opCodes::TSX:
    {
        TSX();
        break;
    }
    case opCodes::LDY_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        LDY(memoryRead(address));
        break;
    }
    case opCodes::LDA_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        LDA(memoryRead(address));
        break;
    }
    case opCodes::LDX_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        LDX(memoryRead(address));
        break;
    }
    case 0xBF:
    {
        //Illegal: LAX_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        LAX(address);
        break;
    }
    case opCodes::CPY_I:
    {
        CPY(currentInstruction.Data1);
        break;
    }
    case opCodes::CMP_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        CMP(memoryRead(address));
        break;
    }
    case 0xC3:
    {
        //Illegal: DCP_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        DCP(address);
        break;
    }
    case opCodes::CPY_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        CPY(memoryRead(address));
        break;
    }
    case opCodes::CMP_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        CMP(memoryRead(address));
        break;
    }
    case opCodes::DEC_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        DEC(address);
        break;
    }
    case 0xC7:
    {
        //Illegal: DCP_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        DCP(address);
        break;
    }
    case opCodes::INY:
    {
        INY();
        break;
    }
    case opCodes::CMP_I:
    {
        CMP(currentInstruction.Data1);
        break;
    }
    case opCodes::DEX:
    {
        DEX();
        break;
    }
    case opCodes::CPY_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        CPY(memoryRead(address));
        break;
    }
    case opCodes::CMP_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        CMP(memoryRead(address));
        break;
    }
    case opCodes::DEC_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        DEC(Utils::joinBytes(ADH,ADL));
        break;
    }
    case 0xCF:
    {
        //Illegal: DCP_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        DCP(address);
        break;
    }
    case opCodes::BNE:
    {
        BNE();
        break;
    }
    case opCodes::CMP_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        CMP(memoryRead(address));
        break;
    }
    case 0xD3:
    {
        //Illegal: DCP_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        DCP(address);
        break;
    }
    case 0xD4:
    {
        //Illegal: DOP_ZP_X
        DOP();
        break;
    }
    case opCodes::CMP_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        CMP(memoryRead(address));
        break;
    }
    case opCodes::DEC_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        DEC(address);
        break;
    }
    case 0xD7:
    {
        //Illegal: DCP_ZP_X
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        DCP(address);
        break;
    }
    case opCodes::CLD:
    {
        CLD();
        break;
    }
    case opCodes::CMP_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        CMP(memoryRead(address));
        break;
    }
    case 0xDB:
    {
        //Illegal: DCP_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        DCP(address);
        break;
    }
    case 0xDA:
    {
        //Illegal: NOP
        NOP();
        break;
    }
    case 0xDC:
    {
        //Illegal: TOP_ABS_X
        TOP();
        break;
    }
    case opCodes::CMP_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        CMP(memoryRead(address));
        break;
    }
    case opCodes::DEC_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        DEC(address);
        break;
    }
    case 0xDF:
    {
        //Illegal: DCP_ABS_X
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        DCP(address);
        break;
    }
    case opCodes::CPX_I:
    {
        CPX(currentInstruction.Data1);
        break;
    }
    case opCodes::SBC_IX:
    {
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        SBC(memoryRead(address));
        break;
    }
    case 0xE3:
    {
        //Illegal: ISC_IX
        Address address = indexedIndirectAddress(currentInstruction.Data1,&X);
        ISC(address);
        break;
    }
    case opCodes::CPX_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        CPX(memoryRead(address));
        break;
    }
    case opCodes::SBC_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        SBC(memoryRead(address));
        break;
    }
    case opCodes::INC_ZP:
    {
        Address address = zeroPageAddress(currentInstruction.Data1);
        INC(address);
        break;
    }
    case 0xE7:
    {
        //Illegal: ISC_ZP
        Address address = zeroPageAddress(currentInstruction.Data1);
        ISC(address);
        break;
    }
    case opCodes::INX:
    {
        INX();
        break;
    }
    case opCodes::SBC_I:
    {
        SBC(currentInstruction.Data1);
        break;
    }
    case opCodes::NOP:
    {
        NOP();
        break;
    }
    case 0xEB:
    {
        //Illegal: SBC_I
        SBC(currentInstruction.Data1);
        break;
    }
    case opCodes::CPX_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        CPX(memoryRead(address));
        break;
    }
    case opCodes::SBC_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        SBC(memoryRead(address));
        break;
    }
    case opCodes::INC_ABS:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        INC(Utils::joinBytes(ADH,ADL));
        break;
    }
    case 0xEF:
    {
        //Illegal: ISC_ABS
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = Utils::joinBytes(ADH,ADL);
        ISC(address);
        break;
    }
    case opCodes::BEQ:
    {
        BEQ();
        break;
    }
    case opCodes::SBC_IY:
    {
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        SBC(memoryRead(address));
        break;
    }
    case 0xF3:
    {
        //Illegal: ISC_IY
        Address address = indirectIndexedAddress(currentInstruction.Data1,&Y);
        ISC(address);
        break;
    }
    case 0xF4:
    {
        //Illegal: DOP_ZP_X
        DOP();
        break;
    }
    case opCodes::SBC_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        SBC(memoryRead(address));
        break;
    }
    case opCodes::INC_ZP_X:
    {
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        INC(address);
        break;
    }
    case 0xF7:
    {
        //Illegal: ISC_ZP_X
        Address address = zeroPageIndexedAddress(currentInstruction.Data1,&X);
        ISC(address);
        break;
    }
    case opCodes::SED:
    {
        SED();
        break;
    }
    case opCodes::SBC_ABS_Y:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        SBC(memoryRead(address));
        break;
    }
    case 0xFA:
    {
        //Illegal: NOP
        NOP();
        break;
    }
    case 0xFB:
    {
        //Illegal ISC_ABS_Y
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&Y);
        ISC(address);
        break;
    }
    case 0xFC:
    {
        //Illegal: TOP_ABS_X
        TOP();
        break;
    }
    case opCodes::SBC_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        SBC(memoryRead(address));
        break;
    }
    case opCodes::INC_ABS_X:
    {
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        INC(address);
        break;
    }
    case 0xFF:
    {
        //Illegal: ISC_ABS_X
        Byte ADL = currentInstruction.Data1;
        Byte ADH = currentInstruction.Data2;
        Address address = absoluteIndexedAddress(ADH,ADL,&X);
        ISC(address);
        break;
    }
    default:
        notImplementedInstruction();
    }
}

void CPU::notImplementedInstruction()
{
    printf("INSTRUCTION NOT IMPLEMENTED: %02X\n\n", memoryRead(pc));
    HLT = true;
}

CPUInstruction CPU::decodeInstruction()
{
    Byte opCode = memoryRead(pc);
    Byte data1;
    Byte data2;

    data1 = memoryRead(pc+1);
    data2 = memoryRead(pc+2);

    switch (opCode){
    case opCodes::ORA_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x03:
    {
        //Illegal: SLO_IX
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x04:
    {
        //Illegal: DOP
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::ORA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::ASL_ZP:
    {
        return CPUInstruction(opCode,data1,5,false);
    }
    case 0x07:
    {
        //Illegal: SLO_ZP
        return CPUInstruction(opCode,data1,5,false);
    }
    case opCodes::PHP:
    {
        return CPUInstruction(opCode,3,false);
    }
    case opCodes::ORA_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::ASL_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case 0x0C:
    {
        //Illegal: TOP
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::ORA_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::ASL_ABS:
    {
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case 0x0F:
    {
        //Illegal: SLO_ABS
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case opCodes::BPL:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(N_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::ORA_IY:
    {
        return CPUInstruction(opCode,data1,5,false); //Según el manual son siempre 5 ciclos, aunque cruces página???
    }
    case 0x13:
    {
        //Illegal: SLO_IY
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x14:
    {
        //Illegal: DOP
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::ORA_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::ASL_ZP_X:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x17:
    {
        //Illegal: SLO_ZP_X
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::CLC:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::ORA_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0x1A:
    {
        //Illegal: NOP
        return CPUInstruction(opCode,2,false);
    }
    case 0x1B:
    {
        //Illegal: SLO_ABS_Y
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x1C:
    {
        //Illegal: TOP_ABS_X
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::ORA_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::ASL_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x1F:
    {
        //Illegal: SLO_ABS_X
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case opCodes::JSR:
    {
        return CPUInstruction(opCode,data1,data2,6,true);
    }
    case opCodes::AND_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x23:
    {
        //Illegal: RLA_IX
        return CPUInstruction(opCode,data1,8,false);
    }
    case opCodes::BIT_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::AND_ZP:
    {
      return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::ROL_ZP:
    {
        return CPUInstruction(opCode,data1,5,false);
    }
    case 0x27:
    {
        //Illegal: RLA_ZP
        return CPUInstruction(opCode,data1,5,false);
    }
    case opCodes::PLP:
    {
        return CPUInstruction(opCode,4,false);
    }
    case opCodes::AND_I:
    {
        return  CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::ROL_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::BIT_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::AND_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::ROL_ABS:
    {
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case 0x2F:
    {
        //Illegal: RLA_ABS
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case opCodes::BMI:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!N_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::AND_IY:
    {
        return CPUInstruction(opCode,data1,5,false); //Parece que son 5 aunque cruces página???
    }
    case 0x33:
    {
        //Illegal: RLA_IY
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x34:
    {
        //Illegal: DOP_ZP_X
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::AND_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::ROL_ZP_X:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x37:
    {
        //Illegal: RLA_ZP_X
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::SEC:
    {
        return  CPUInstruction(opCode,2,false);
    }
    case opCodes::AND_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0x3A:
    {
        //Illegal: NOP
        return CPUInstruction(opCode,2,false);
    }
    case 0x3B:
    {
        //Illegal: RLA_ABS_Y
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x3C:
    {
        //Illegal: TOP_ABS_X
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::AND_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::ROL_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x3F:
    {
        //Illegal: RLA_ABS_X
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case opCodes::RTI:
    {
        return CPUInstruction(opCode,6,true);
    }
    case opCodes::EOR_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x43:
    {
        //Illegal: SRE_IX
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x44:
    {
        //Illegal: DOP_ZP
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::EOR_ZP:
    {
      return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::LSR_ZP:
    {
        return CPUInstruction(opCode,data1,5,false);
    }
    case 0x47:
    {
        //Illegal: SRE_ZP
        return CPUInstruction(opCode,data1,5,false);
    }
    case opCodes::PHA:
    {
        return CPUInstruction(opCode,3,false);
    }
    case opCodes::EOR_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::LSR_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::JMP_ABS:
    {
        return CPUInstruction(opCode,data1,data2,3,true);
    }
    case opCodes::EOR_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::LSR_ABS:
    {
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case 0x4F:
    {
        //Illegal: SRE_ABS
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case opCodes::BVC:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(V_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::EOR_IY:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = indirectIndexedAddress(data1,&temp);
        Address addressWithIndex = indirectIndexedAddress(data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,5,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,6,false);
    }
    case 0x53:
    {
        //Illegal: SRE_IY
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x54:
    {
        //Illegal: DOP_ZP_X
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::EOR_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::LSR_ZP_X:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x57:
    {
        //Illegal: SRE_ZP_X
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::EOR_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0x5A:
    {
        //Illegal: NOP
        return CPUInstruction(opCode,2,false);
    }
    case 0x5B:
    {
        //Illegal: SRE_ABS_Y
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x5C:
    {
        //Illegal: TOP_ABS_X
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::EOR_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::LSR_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x5F:
    {
        //Illegal: SRE_ABS_X
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case opCodes::RTS:
    {
        return CPUInstruction(opCode,6,true);
    }
    case opCodes::ADC_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x63:
    {
        //Illegal: RRA_IX
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x64:
    {
        //Illegal: DOP_ZP
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::ADC_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::ROR_ZP:
    {
        return CPUInstruction(opCode,data1,5,false);
    }
    case 0x67:
    {
        //Illegal: RRA_ZP
        return CPUInstruction(opCode,data1,5,false);
    }
    case opCodes::PLA:
    {
        return CPUInstruction(opCode,4,false);
    }
    case opCodes::ADC_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::ROR_A:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::JMP_I:
    {
        return CPUInstruction(opCode,data1,data2,5,true);
    }
    case opCodes::ADC_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::ROR_ABS:
    {
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case 0x6F:
    {
        //Illegal: RRA_ABS
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case opCodes::BVS:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!V_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::ADC_IY:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = indirectIndexedAddress(data1,&temp);
        Address addressWithIndex = indirectIndexedAddress(data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,5,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,6,false);
    }
    case 0x73:
    {
        //Illegal: RRA_IY
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0x74:
    {
        //Illegal: DOP_ZP_X
        return CPUInstruction(opCode,data1,4,false);
    }
    case 0x7C:
    {
        //Illegal: TOP_ABS_X
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::ADC_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::ROR_ZP_X:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x77:
    {
        //Illegal: RRA_ZP_X
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::SEI:
    {
        return  CPUInstruction(opCode,2,false);
    }
    case opCodes::ADC_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0x7A:
    {
        //Illegal: NOP
        return CPUInstruction(opCode,2,false);
    }
    case 0x7B:
    {
        //Illegal: RRA_ABS_Y
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case opCodes::ADC_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::ROR_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x7F:
    {
        //Illegal: RRA_ABS_X
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0x80:
    {
        //Illegal: DOP
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::STA_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0x83:
    {
        //Illegal: SAX_IX
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::STY_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::STA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::STX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case 0x87:
    {
        //Illegal: SAX_ZP
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::DEY:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::TXA:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::STY_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::STA_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::STX_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case 0x8F:
    {
        //Illegal: SAX_ABS
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::BCC:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(C_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::STA_IY:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::STY_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::STA_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::STX_ZP_Y:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case 0x97:
    {
        //Illegal: SAX_ZP_Y
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::TYA:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::STA_ABS_Y:
    {
        return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::TXS:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::STA_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::LDY_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::LDA_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::LDX_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case 0xA3:
    {
        //Illegal: LAX_IX
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::LDY_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::LDA_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::LDX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case 0xA7:
    {
        //Illegal: LAX_ZP
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::TAY:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::LDA_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::TAX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::LDY_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::LDA_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::LDX_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case 0xAF:
    {
        //Illegal: LAX_ABS
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::BCS:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!C_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::LDA_IY:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = indirectIndexedAddress(data1,&temp);
        Address addressWithIndex = indirectIndexedAddress(data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,5,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,6,false);
    }
    case 0xB3:
    {
        //Illegal: LAX_IY
        Register8 temp = 0;
        Address addressWithoutIndex = indirectIndexedAddress(data1,&temp);
        Address addressWithIndex = indirectIndexedAddress(data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,5,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::LDY_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::LDA_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::LDX_ZP_Y:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case 0xB7:
    {
        //Illegal: LAX_ZP_Y
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::CLV:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::LDA_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::TSX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::LDY_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::LDA_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::LDX_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0xBF:
    {
        //Illegal: LAX_ABS_Y
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::CPY_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::CMP_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0xC3:
    {
        //Illegal: DCP_IX
        return CPUInstruction(opCode,data1,8,false);
    }
    case opCodes::CPY_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::CMP_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::DEC_ZP:
    {
        return CPUInstruction(opCode,data1,5,false);
    }
    case 0xC7:
    {
        //Illegal: DCP_ZP
        return CPUInstruction(opCode,data1,5,false);
    }
    case opCodes::INY:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::CMP_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::DEX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::CPY_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::CMP_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::DEC_ABS:
    {
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case 0xCF:
    {
        //Illegal: DCP_ABS
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case opCodes::BNE:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(Z_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::CMP_IY:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = indirectIndexedAddress(data1,&temp);
        Address addressWithIndex = indirectIndexedAddress(data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,5,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,6,false);
    }
    case 0xD3:
    {
        //Illegal: DCP_IY
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0xD4:
    {
        //Illegal: DOP_ZP_X
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::CMP_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::DEC_ZP_X:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0xD7:
    {
        //Illegal: DCP_ZP_X
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::CLD:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::CMP_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0xDB:
    {
        //Illegal: DCP_ABS_Y
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0xDA:
    {
        //Illegal: NOP
        return CPUInstruction(opCode,2,false);
    }
    case 0xDC:
    {
        //Illegal: TOP_ABS_X
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::CMP_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::DEC_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0xDF:
    {
        //Illegal: DCP_ABS_X
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case opCodes::CPX_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::SBC_IX:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0xE3:
    {
        //Illegal: ISC_ABS_IX
        return CPUInstruction(opCode,data1,8,false);
    }
    case opCodes::CPX_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::SBC_ZP:
    {
        return CPUInstruction(opCode,data1,3,false);
    }
    case opCodes::INC_ZP:
    {
        return CPUInstruction(opCode,data1,5,false);
    }
    case 0xE7:
    {
        //Illegal: ISC_ZP
        return CPUInstruction(opCode,data1,5,false);
    }
    case opCodes::INX:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::SBC_I:
    {
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::NOP:
    {
        return CPUInstruction(opCode,2,false);
    }
    case 0xEB:
    {
        //Illegal: SBC_I
        return CPUInstruction(opCode,data1,2,false);
    }
    case opCodes::CPX_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::SBC_ABS:
    {
        return CPUInstruction(opCode,data1,data2,4,false);
    }
    case opCodes::INC_ABS:
    {
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case 0xEF:
    {
        //Illegal: ISC_ABS
        return CPUInstruction(opCode,data1,data2,6,false);
    }
    case opCodes::BEQ:
    {
        //If no branch occurs, the instruction only takes 2 cycles
        if(!Z_FlagSet())
        {
            return  CPUInstruction(opCode,data1,2,false);
        }

        Address newAddress = relativeAddress(data1) + 2;

        if(samePage(pc+2,newAddress))
            //If the branch occurs to the same page, the instruction takes 3 cycles
            return CPUInstruction(opCode,data1,3,false);
        else
            //If the branch occurs to another page, the instruction takes 4 cycles
            return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::SBC_IY:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = indirectIndexedAddress(data1,&temp);
        Address addressWithIndex = indirectIndexedAddress(data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,5,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,6,false);
    }
    case 0xF3:
    {
        //Illegal: ISC_IY
        return CPUInstruction(opCode,data1,8,false);
    }
    case 0xF4:
    {
        //Illegal: DOP_ZP_X
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::SBC_ZP_X:
    {
        return CPUInstruction(opCode,data1,4,false);
    }
    case opCodes::INC_ZP_X:
    {
        return CPUInstruction(opCode,data1,6,false);
    }
    case 0xF7:
    {
        //Illegal: ISC_ZP_X
        return CPUInstruction(opCode,data1,6,false);
    }
    case opCodes::SED:
    {
        return CPUInstruction(opCode,2,false);
    }
    case opCodes::SBC_ABS_Y:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&Y);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case 0xFA:
    {
        //Illegal: NOP
        return CPUInstruction(opCode,2,false);
    }
    case 0xFB:
    {
        //Illegal: ISC_ABS_Y
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0xFC:
    {
        //Illegal: TOP_ABS_X
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::SBC_ABS_X:
    {
        Register8 temp = 0;
        Address addressWithoutIndex = absoluteIndexedAddress(data2,data1,&temp);
        Address addressWithIndex = absoluteIndexedAddress(data2,data1,&X);

        if(samePage(addressWithIndex,addressWithoutIndex))
            return CPUInstruction(opCode,data1,data2,4,false);
        else
            //If a page is crossed when calculating the address, the instruction takes one extra cycle
            return CPUInstruction(opCode,data1,data2,5,false);
    }
    case opCodes::INC_ABS_X:
    {
        return CPUInstruction(opCode,data1,data2,7,false);
    }
    case 0xFF:
    {
        //Illegal: ISC_ABS_X
        return CPUInstruction(opCode,data1,data2,7,false);
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

/*Addressing modes*/
Address CPU::zeroPageAddress(Byte ADL)
{
    return Utils::joinBytes(0x00,ADL);
}

Address CPU::relativeAddress(Byte Offset)
{
    int8_t signedOffset = (int8_t) Offset;
    return pc + signedOffset;
}

Address CPU::zeroPageIndexedAddress(Byte ADL, Register8 *index)
{
    return zeroPageAddress(ADL + *index);
}

Address CPU::indirectAddress(Byte IAH, Byte IAL)
{
    Byte ADL = memoryRead(Utils::joinBytes(IAH,IAL));
    Byte ADH = memoryRead(Utils::joinBytes(IAH,IAL+1)); //This instruction can not cross a page when calculating ADH, so only IAL is incremented. REF: http://www.6502.org/tutorials/6502opcodes.html#JMP
    return Utils::joinBytes(ADH,ADL);
}

Address CPU::absoluteIndexedAddress(Byte ADH, Byte ADL, Register8 *index)
{
    return Utils::joinBytes(ADH,ADL) + *index;
}

Address CPU::indexedIndirectAddress(Byte zp_ADL, Register8 *index)
{
    Address zp_Address1 = zeroPageAddress(zp_ADL + *index);
    Address zp_Address2 = zeroPageAddress(zp_ADL + *index + 1);
    Byte ADL = memoryRead(zp_Address1);
    Byte ADH = memoryRead(zp_Address2);
    return Utils::joinBytes(ADH,ADL);
}

Address CPU::indirectIndexedAddress(Byte zp_ADL, Register8 *index)
{
    Address zp_Address1 = zeroPageAddress(zp_ADL);
    Address zp_Address2 = zeroPageAddress(zp_ADL+1);
    Byte ADL = memoryRead(zp_Address1);
    Byte ADH = memoryRead(zp_Address2);
    return Utils::joinBytes(ADH, ADL) + *index;
}

bool CPU::samePage(Address address1, Address address2)
{
    return (Utils::highByte(address1) == Utils::highByte(address2));
}


/*Flags*/
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

bool CPU::operationHasOverflow(Byte a, Byte b, Byte result)
{
    Byte signA = (a >> 7);
    Byte signOperand = (b >> 7);
    Byte signResult = (result >> 7);

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

/*Stack*/
void CPU::pushToStack_2Bytes(Address data)
{
    pushToStack_1Byte(Utils::highByte(data));
    pushToStack_1Byte(Utils::lowByte(data));
}

void CPU::pushToStack_1Byte(Byte data)
{
    memoryWrite(data,Utils::joinBytes(0x01,sp),false);
    sp--;
}

Address CPU::pullFromStack_2Bytes()
{
    Byte LByte = pullFromStack_1Byte();
    Byte HByte = pullFromStack_1Byte();
    return Utils::joinBytes(HByte,LByte);
}

Byte CPU::pullFromStack_1Byte()
{
    sp +=1;
    return memoryRead(Utils::joinBytes(0x01,sp));
}

/*Registers*/
void CPU::loadRegister(Register8 *Reg, Byte value)
{
    *Reg = value;
    set_Z_Flag(*Reg == 0);
    set_N_Flag(*Reg >> 7);
}

/*Memory*/
void CPU::memoryWrite(Byte value, Address address, bool checkFlags)
{
    if(checkFlags)
    {
        set_Z_Flag(value == 0);
        set_N_Flag(value >> 7);
    }

    //RAM
    if (address <=0x1FFF)
    {
        //printf("Escritura [%04X] --> RAM[%04X]\n", address, (address & 0x07FF));
        RAM[address & 0x07FF] = value;
    }

    //PPU Registers
    if (address >= 0x2000 && address <= 0x3FFF)
    {
        //printf("Escritura [%04X] --> PPU_Register[%04X]\n", address, (address & 0x0007));
        PPU_Register[address & 0x0007] = value;
    }

    //APU I/O
    if(address >= 0x4000 && address <= 0x4017)
    {
        //printf("Escritura [%04X] --> APU_IO[%04X]\n", address, (address & 0x001F));
        APU_IO[address & 0x001F] = value;
    }

    //APU Test
    if(address >= 0x4018 && address <= 0x401F)
    {
        //printf("Escritura [%04X] --> APU_Test[%04X]\n", address, (address & 0x0007));
        APU_Test[address  & 0x0007] = value;
    }

    //Cartridge
    if(address >= 0x4020)
    {
        printf("Escritura [%04X] --> Cartridge[%04X] ||| ATENCION: Se ha intentado escribir en la ROM", address, (address - (Address) 0x4020));
        //Cartridge[address - (Address) 0x4020] = value;
    }
}

Byte CPU::memoryRead(Address address)
{
    //RAM
    if (address <=0x1FFF)
    {
        //printf("Lectura [%04X] --> RAM[%04X]\n", address, (address & 0x07FF));
        return RAM[address & 0x07FF];
    }

    //PPU Registers
    if (address >= 0x2000 && address <= 0x3FFF)
    {
        printf("Lectura [%04X] --> PPU_Register[%04X]\n", address, (address & 0x0007));
        return PPU_Register[address & 0x0007];
    }

    //APU I/O
    if(address >= 0x4000 && address <= 0x4017)
    {
        printf("Lectura [%04X] --> APU_IO[%04X]\n", address, (address & 0x001F));
        return APU_IO[address & 0x001F];
    }

    //APU Test
    if(address >= 0x4018 && address <= 0x401F)
    {
        printf("Lectura [%04X] --> APU_Test[%04X]\n", address, (address & 0x0007));
        return APU_Test[address  & 0x0007];
    }

    //Cartridge
    if(address >= 0x4020)
    {
        //printf("Lectura [%04X] --> Cartridge[%04X]\n", address, (address - (Address) 0x4020));
        return Cartridge[address - (Address) 0x4020];
    }
    printf("Error, direccion de memoria no controlada: %04X\n", address);
    return 0x00; //Nunca debería llegar aquí.
}

/*Official Instructions*/
void CPU::ADC(Byte operand)
{
    uint16_t result = A + operand + (Byte)C_FlagSet();
    set_C_Flag(result > 0xFF);
    set_V_Flag(operationHasOverflow(A,operand,result));
    loadRegister(&A,result);                            //loadRegister function already considers N and Z flags.
}

void CPU::AND(Byte value)
{
    Byte result = A & value;
    loadRegister(&A,result);
}

void CPU::ASL(Address address)
{
    Byte value = memoryRead(address);
    set_C_Flag(value & 0b10000000);
    Byte shiftedValue = (value << 1);
    memoryWrite(shiftedValue,address,true);
}

void CPU::BCC()
{
    if(!C_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BCS()
{
    if(C_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BEQ()
{
    if(Z_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BIT(Byte value)
{
    set_N_Flag(value >> 7);
    set_V_Flag((value & 0b01000000) >> 6);

    value &= A;

    set_Z_Flag(value == 0);
}

void CPU::BMI()
{
    if(N_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BNE()
{
    if(!Z_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BPL()
{
    if(!N_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BRK()
{
    notImplementedInstruction();
}

void CPU::BVC()
{
    if(!V_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::BVS()
{
    if(V_FlagSet())
    {
        Address newAddress = relativeAddress(currentInstruction.Data1);
        pc = newAddress;
    }
}

void CPU::CLC()
{
    set_C_Flag(false);
}

void CPU::CLD()
{
    set_D_Flag(false);
}

void CPU::CLI()
{
    notImplementedInstruction();
}

void CPU::CLV()
{
    set_V_Flag(false);
}

void CPU::CMP(Byte value)
{
    Byte result = A - value;
    set_Z_Flag(result == 0);
    set_N_Flag(result >> 7);
    set_C_Flag(value <= A);
}

void CPU::CPX(Byte value)
{
    Byte result = X - value;
    set_Z_Flag(result == 0);
    set_N_Flag(result >> 7);
    set_C_Flag(X >= value);
}

void CPU::CPY(Byte value)
{
    Byte result = Y - value;
    set_Z_Flag(result == 0);
    set_N_Flag(result >> 7);
    set_C_Flag(Y >= value);
}

void CPU::DEC(Address address)
{
    Byte value = memoryRead(address);
    memoryWrite(value-1,address,true);
}

void CPU::DEX()
{
    Byte result = X-1;
    loadRegister(&X,result);
}

void CPU::DEY()
{
    Byte result = Y-1;
    loadRegister(&Y,result);
}

void CPU::EOR(Byte value)
{
    Byte result = A ^ value;
    loadRegister(&A,result);
}

void CPU::INC(Address address)
{
    Byte value = memoryRead(address);
    memoryWrite(value+1,address,true);
}

void CPU::INX()
{
    Byte result = X+1;
    loadRegister(&X,result);
}

void CPU::INY()
{
    Byte result = Y+1;
    loadRegister(&Y,result);
}

void CPU::JMP(Address address)
{
    pc = address;
}

void CPU::JSR()
{
    Byte ADL = currentInstruction.Data1;
    Byte ADH = currentInstruction.Data2;

    pc +=2;

    pushToStack_2Bytes(pc);

    pc = Utils::joinBytes(ADH,ADL);
}

void CPU::LDA(Byte value)
{
    loadRegister(&A,value);
}

void CPU::LDX(Byte value)
{
    loadRegister(&X,value);
}

void CPU::LDY(Byte value)
{
    loadRegister(&Y,value);
}

void CPU::LSR(Address address)
{
    Byte value = memoryRead(address);
    set_C_Flag(value & 0b00000001);
    Byte shiftedValue = (value >> 1);
    memoryWrite(shiftedValue,address,true);
}

void CPU::NOP()
{
    //No operation
}

void CPU::ORA(Byte value)
{
    Byte result = A | value;
    loadRegister(&A,result);
}

void CPU::PHA()
{
    pushToStack_1Byte(A);
}

void CPU::PHP()
{
    //Bit 4 doesn't exist on the status register, only when it is pushed to the stack.  Ref: https://www.nesdev.org/wiki/Status_flags
    Byte statusRegister = (P | 0b00010000);          //Bit4 = 1
    pushToStack_1Byte(statusRegister);
}

void CPU::PLA()
{
    Byte value = pullFromStack_1Byte();
    loadRegister(&A,value);
}

void CPU::PLP()
{
    //Bit 4 doesn't exist on the status register, only when it is pushed to the stack. Bit 5 is always high.  Ref: https://www.nesdev.org/wiki/Status_flags
    Register8 statusRegister = (pullFromStack_1Byte() & 0b11101111);  //Bit4 = 0
    statusRegister |= 0b00100000;                                     //Bit5 = 1
    P = statusRegister;
}

void CPU::ROL(Address address)
{
    Byte value = memoryRead(address);
    Byte shiftedValue = (value << 1) | C_FlagSet();
    set_C_Flag(value & 0b10000000);
    memoryWrite(shiftedValue,address,true);
}

void CPU::ROR(Address address)
{
    Byte value = memoryRead(address);
    Byte C = C_FlagSet();
    Byte shiftedValue = (value >> 1) | (C << 7);
    set_C_Flag(value & 0b00000001);
    memoryWrite(shiftedValue,address,true);
}

void CPU::RTI()
{
    PLP();
    pc = pullFromStack_2Bytes();
}

void CPU::RTS()
{
    pc = pullFromStack_2Bytes();
    pc++;
}

void CPU::SBC(Byte operand)
{
    ADC(~operand);
}

void CPU::SEC()
{
    set_C_Flag(true);
}

void CPU::SED()
{
    set_D_Flag(true);
}

void CPU::SEI()
{
    set_I_Flag(true);
}

void CPU::STA(Address address)
{
    memoryWrite(A,address,false);
}

void CPU::STX(Address address)
{
    memoryWrite(X,address,false);
}

void CPU::STY(Address address)
{
    memoryWrite(Y,address,false);
}

void CPU::TAX()
{
    loadRegister(&X,A);
}

void CPU::TAY()
{
    loadRegister(&Y,A);
}

void CPU::TSX()
{
    loadRegister(&X,sp);
}

void CPU::TXA()
{
    loadRegister(&A,X);
}

void CPU::TXS()
{
    sp = X;
}

void CPU::TYA()
{
    loadRegister(&A,Y);
}

/*Illegal Instructions*/
void CPU::DCP(Address address)
{
    DEC(address);
    CMP(memoryRead(address));
}

void CPU::DOP()
{
    NOP();
}

void CPU::ISC(Address address)
{
    INC(address);
    SBC(memoryRead(address));
}

void CPU::LAX(Address address)
{
    Byte value = memoryRead(address);
    LDA(value);
    LDX(value);
}

void CPU::RLA(Address address)
{
    ROL(address);
    AND(memoryRead(address));
}

void CPU::RRA(Address address)
{
    ROR(address);
    ADC(memoryRead(address));
}

void CPU::SAX(Address address)
{
    Byte result = X & A;
    memoryWrite(result,address,false); //En la docu pone que checkFlags = true ???? pero así funciona.
}

void CPU::SLO(Address address)
{
    ASL(address);
    ORA(memoryRead(address));
}

void CPU::SRE(Address address)
{
    LSR(address);
    EOR(memoryRead(address));
}

void CPU::TOP()
{
    NOP();
}










