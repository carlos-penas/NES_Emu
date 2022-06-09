#include "bus.h"
#include "stdio.h"
#include <cstring>

Bus::Bus()
{
    memset(RAM,0,sizeof(RAM));
    memset(PPU_Register,0,sizeof(PPU_Register));
    memset(APU_IO,0,sizeof(APU_IO));
    memset(APU_Test,0,sizeof(APU_Test));
}

void Bus::connectCartridge(Cartridge *cartridge)
{
    this->cartridge = cartridge;
//    printf("Primera instrucción: %02x\n", Read(0xC000));
//    printf("Luego: %02x\n", Read(0xC001));
//    printf("Luego: %02x\n", Read(0xC002));
//    printf("Últimas: %02x\n", Read(0xFFFA));
//    printf("Últimas: %02x\n", Read(0xFFFB));
//    printf("Últimas: %02x\n", Read(0xFFFC));
//    printf("Últimas: %02x\n", Read(0xFFFD));
//    printf("Últimas: %02x\n", Read(0xFFFE));
//    printf("Últimas: %02x\n", Read(0xFFFF));
}

void Bus::Write(Byte value, Address address)
{
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
        cartridge->CPU_Write(value,address);
    }
}

Byte Bus::Read(Address address)
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
        return cartridge->CPU_Read(address);
    }
    printf("Error, direccion de memoria no controlada: %04X\n", address);
    return 0x00; //Nunca debería llegar aquí.
}
