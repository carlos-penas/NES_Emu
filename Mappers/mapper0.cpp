#include "mapper0.h"
#include <stdio.h>

#ifdef COMPILE_WINDOWS
#include "../Exceptions/notmappedaddressexception.h"
#endif
#ifndef COMPILE_WINDOWS
#include "Exceptions/notmappedaddressexception.h"
#endif


Mapper0::Mapper0(vector<Register8> PRG_ROM, uint16_t PRG_RAM_Size, vector<Register8> CHR_ROM)
    : Mapper(PRG_ROM,CHR_ROM)
{
    if(PRG_RAM_Size > 0)
        PRG_RAM.resize(PRG_RAM_Size);
}

void Mapper0::CPU_Write(Byte value, Address address)
{
    if(address >= 0x6000 && address <= 0x7FFF)
    {
        if(PRG_RAM.size() == 2048)  // 2 KB
        {
            printf("Escritura [%04X] --> PRG_RAM[%04X]\n", address, address & 0x7FF);
            PRG_RAM[address & 0x7FF] = value;
        }
        else if(PRG_RAM.size() == 4096) // 4 KB
        {
            printf("Escritura [%04X] --> PRG_RAM[%04X]\n", address, address & 0xFFF);
            PRG_RAM[address & 0xFFF] = value;
        }
        else
        {
            //PRG RAM not defined
            //throw NotMappedAddressException(address,WriteAttempt);
        }
    }
    else if(address >= 0x8000)
    {
        if(PRG_ROM.size() == 16384)  // 16 KB
        {
            printf("Escritura [%04X] --> PRG_ROM[%04X] ||| ATENCION: Se ha intentado escribir  en PRG_ROM\n", address, address & 0x3FFF);
            //PRG_ROM[address & 0x3FFF] = value;
        }
        else if(PRG_ROM.size() == 32768)  // 32 KB
        {
            printf("Escritura [%04X] --> PRG_ROM[%04X] ||| ATENCION: Se ha intentado escribir  en PRG_ROM\n", address, address & 0x7FFF);
            //PRG_ROM[address & 0x7FFF] = value;
        }
        else
        {
            //throw NotMappedAddressException(address,WriteAttempt);
        }
    }
    else
    {
        //throw NotMappedAddressException(address,WriteAttempt);
    }
}

Byte Mapper0::CPU_Read(Address address)
{
    if(address >= 0x6000 && address <= 0x7FFF)
    {
        if(PRG_RAM.size() == 2048)  // 2 KB
        {
            printf("Lectura [%04X] --> PRG_RAM[%04X]\n", address, address & 0x7FF);
            return PRG_RAM[address & 0x7FF];
        }
        else if(PRG_RAM.size() == 4096) // 4 KB
        {
            printf("Lectura [%04X] --> PRG_RAM[%04X]\n", address, address & 0xFFF);
            return PRG_RAM[address & 0xFFF];
        }
        else
        {
            //PRG RAM not defined
            //throw NotMappedAddressException(address,ReadAttempt);
            return 0x00;
        }
    }
    else if(address >= 0x8000)
    {
        if(PRG_ROM.size() == 16384)  // 16 KB
        {
            //printf("Lectura [%04X] --> PRG_ROM[%04X]\n", address, address & 0x3FFF);
            return PRG_ROM[address & 0x3FFF];
        }
        else if(PRG_ROM.size() == 32768)  // 32 KB
        {
            //printf("Lectura [%04X] --> PRG_ROM[%04X]\n", address, address & 0x7FFF);
            return PRG_ROM[address & 0x7FFF];
        }
        else
        {
            //throw NotMappedAddressException(address,ReadAttempt);
            return 0x00;
        }
    }
    else
    {
        //throw NotMappedAddressException(address,ReadAttempt);
        return 0x00;
    }
}

Byte Mapper0::PPU_Read(Address address)
{
    return CHR_ROM[address];
}
