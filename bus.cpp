#include "bus.h"
#include "stdio.h"
#include <cstring>
#include "compilationSettings.h"

#ifdef COMPILE_WINDOWS
#include "SFML/Window.hpp"
#else
#include "Window.hpp"
#endif

Bus::Bus()
{
    memset(RAM,0,sizeof(RAM));
    memset(APU_IO,0,sizeof(APU_IO));
    memset(APU_Test,0,sizeof(APU_Test));

    controllers[0] = 0;
    controllers[1] = 0;
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
        if(address >= 0x4016)
        {
            pollControllerInput(address - 0x4016);
        }
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
    else if (address >= 0x2000 && address <= 0x3FFF)
    {
#ifdef PRINTLOG
        printf("Lectura [%04X] --> PPU_Register[%04X]\n", address, (address & 0x2007));
#endif
        return ppu->cpuRead(address & 0x2007);
    }

    //APU I/O
    else if(address >= 0x4000 && address <= 0x4017)
    {
#ifdef PRINTLOG
        printf("Lectura [%04X] --> APU_IO[%04X]\n", address, (address & 0x001F));
#endif
        if(address >= 0x4016)
        {
            return readControllerData(address - 0x4016);
        }
        else
        {
            return APU_IO[address & 0x001F];
        }
    }

    //APU Test
    else if(address >= 0x4018 && address <= 0x401F)
    {
#ifdef PRINTLOG
        printf("Lectura [%04X] --> APU_Test[%04X]\n", address, (address & 0x0007));
#endif
        return APU_Test[address  & 0x0007];
    }

    //Cartridge
    else if(address >= 0x4020)
    {
        return cartridge->CPU_Read(address);
    }
    printf("Error, direccion de memoria no controlada: %04X\n", address);
    return 0x00; //Nunca debería llegar aquí.
}

void Bus::pollControllerInput(int i)
{
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        controllers[i] |= 0x80;
    }
    else
    {
        controllers[i] &= 0x7F;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        controllers[i] |= 0x40;
    }
    else
    {
        controllers[i] &= 0xBF;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        controllers[i] |= 0x20;
    }
    else
    {
        controllers[i] &= 0xDF;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        controllers[i] |= 0x10;
    }
    else
    {
        controllers[i] &= 0xEF;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
    {
        controllers[i] |= 0x8;
    }
    else
    {
        controllers[i] &= 0xF7;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::Tab))
    {
        controllers[i] |= 0x4;
    }
    else
    {
        controllers[i] &= 0xFB;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::M))
    {
        controllers[i] |= 0x2;
    }
    else
    {
        controllers[i] &= 0xFD;
    }
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::K))
    {
        controllers[i] |= 0x1;
    }
    else
    {
        controllers[i] &= 0xFE;
    }
}

Byte Bus::readControllerData(int i)
{
    Byte data = controllers[i] & 0x01;
    controllers[i] >>= 1;
    return data;
}
