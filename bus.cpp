#include "bus.h"
#include "stdio.h"
#include <cstring>

Bus::Bus()
{
    memset(RAM,0,sizeof(RAM));
    memset(APU_IO,0,sizeof(APU_IO));
    memset(APU_Test,0,sizeof(APU_Test));
}

void Bus::connectCartridge(Cartridge *cartridge)
{
    this->cartridge = cartridge;
}

void Bus::disconnectCartridge()
{
    this->cartridge = NULL;
}

void Bus::connectPPU(PPU *ppu)
{
    this->ppu = ppu;
}

void Bus::disconnectPPU()
{
    this->ppu = NULL;
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
    else if (address >= 0x2000 && address <= 0x3FFF)
    {
#ifdef PRINTLOG
        printf("Escritura [%04X] --> PPU_Register[%04X]\n", address, (address & 0x2007));
#endif
        ppu->cpuWrite(value, address & 0x2007);
    }

    //APU I/O
    else if(address >= 0x4000 && address <= 0x4017)
    {
#ifdef PRINTLOG
        printf("Escritura [%04X] --> APU_IO[%04X]\n", address, (address & 0x001F));
#endif
        APU_IO[address & 0x001F] = value;

        if(address == 0x4014)
            OAM_DMA_Transfer(value);
    }

    //APU Test
    else if(address >= 0x4018 && address <= 0x401F)
    {
        //printf("Escritura [%04X] --> APU_Test[%04X]\n", address, (address & 0x0007));
        APU_Test[address  & 0x0007] = value;
    }

    //Cartridge
    else if(address >= 0x4020)
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
#ifdef PRINTLOG
        printf("Lectura [%04X] --> PPU_Register[%04X]\n", address, (address & 0x2007));
#endif
        return ppu->cpuRead(address & 0x2007);
    }

    //APU I/O
    if(address >= 0x4000 && address <= 0x4017)
    {
#ifdef PRINTLOG
        printf("Lectura [%04X] --> APU_IO[%04X]\n", address, (address & 0x001F));
#endif
        return APU_IO[address & 0x001F];
    }

    //APU Test
    if(address >= 0x4018 && address <= 0x401F)
    {
#ifdef PRINTLOG
        printf("Lectura [%04X] --> APU_Test[%04X]\n", address, (address & 0x0007));
#endif
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

void Bus::OAM_DMA_Transfer(Byte ADH)
{
    Address address = Utils::joinBytes(ADH,00);
    for(int i = 0; i < 256; i++)
    {
        Byte value = Read(address + i);
        ppu->OAM_DMA_Transfer(value,i);
    }
}
