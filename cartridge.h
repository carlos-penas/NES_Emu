#ifndef CARTRIDGE_H
#define CARTRIDGE_H
#include <types.h>
#include <string>
#include <vector>
#include "Mappers/mapper.h"

using namespace std;

enum HeaderType{
    v1 = 1,
    v2
};


struct NES_10_Header{
    char signature [4];
    Byte PRG_ROM_Size;
    Byte CHR_ROM_Size;
    Byte Flags6;
    Byte Flags7;
    Byte PRG_RAM_Size;
    Byte TV_System;
    Byte Flags10;
    Byte padding [5];
};

struct NES_20_Header{
    char signature [4];
    Byte PRG_ROM_Size_LSB;
    Byte CHR_ROM_Size_LSB;
    Byte Flags6;
    Byte Flags7;
    Byte Mapper_MSB;
    Byte ROM_MSB;
    Byte PRG_RAM_Size;
    Byte CHR_RAM_Size;
    Byte TV_System;
    Byte Vs_SystemType;
    Byte MiscellaneousROMs;
    Byte ExpansionDevice;
};

union NES_Header{
  NES_10_Header v1;
  NES_20_Header v2;
};


class Cartridge
{
public:
    Cartridge();
    bool loadROM(string path);
    void CPU_Write(Byte value, Address address);
    Byte CPU_Read(Address address);
    Byte* PPU_GetPattern(uint16_t index);
    Byte PPU_Read(Address address);
private:
    HeaderType headerType;
    Byte mapperId;
    Mapper *mapper;
};

#endif // CARTRIDGE_H
