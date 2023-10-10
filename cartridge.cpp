#include "cartridge.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include "Mappers/mapper0.h"
#include "Mappers/mapper2.h"

Cartridge::Cartridge()
{

}

Cartridge::~Cartridge()
{
    delete mapper;
    mapper = NULL;
}

bool Cartridge::loadROM(string path)
{
    ifstream file(path, ios_base::in | ios_base::binary);
    NES_Header header;

    uint16_t PRG_RAM_Size = 0;
    uint64_t PRG_ROM_Size = 0;
    uint64_t CHR_ROM_Size = 0;

    //Read file header (16 Bytes)
    file.read((char*)&header,sizeof(header));

    //Verify NES file signature
    if(header.v1.signature[0] != 'N' || header.v1.signature[1] != 'E' || header.v1.signature[2] != 'S' || header.v1.signature[3] != 0x1A)
        return false;

    //Identify header type
    if((header.v1.Flags7 & 0x0C) == 0x08)
        headerType = v2;
    else if((header.v1.Flags7 & 0x0C) == 0x00)
        headerType = v1;
    else
        return false;

    //INES 1.0
    if(headerType == v1)
    {
        //Identify the mapper
        mapperId = (header.v1.Flags6 >> 4) | (header.v1.Flags7 & 0xF0);

        //Get PRG RAM Size if defined
        if(header.v1.Flags6 & 0x02)
        {
            PRG_RAM_Size = header.v1.PRG_RAM_Size;
        }

        //Get PRG ROM Size (16 KB units)
        PRG_ROM_Size = 16384 * header.v1.PRG_ROM_Size;

        //Get CHR ROM Size (8 KB units)
        CHR_ROM_Size = 8192 * header.v1.CHR_ROM_Size;
    }
    //INES 2.0
    else if(headerType == v2)
    {
        PRG_ROM_Size = 16384 * header.v2.PRG_ROM_Size_LSB;  //TEMPORAL
    }
    else
        return false;


    vector<Register8> PRG_ROM;
    vector<Register8> CHR_ROM;

    PRG_ROM.resize(PRG_ROM_Size);
    file.read((char*)PRG_ROM.data(),PRG_ROM_Size);

    CHR_ROM.resize(CHR_ROM_Size);
    file.read((char*)CHR_ROM.data(),CHR_ROM_Size);
    file.close();

    printf("Informacion de la ROM:\n");
    printf("\tMapper %d\n",mapperId);
    printf("\tCabecera version %d\n",headerType);
    cout << "\tROM: " << PRG_ROM_Size << " bytes" << endl;
    cout << "\tRAM: " << PRG_RAM_Size << " bytes" << endl;
    cout << "\tCHR: " << CHR_ROM_Size << " bytes" << endl;

    if(mapperId == 0)
    {
        if(PRG_ROM_Size == 16384 || PRG_ROM_Size == 32768)
        {
            mapper = new Mapper0(PRG_ROM, PRG_RAM_Size, CHR_ROM);
        }
        else
        {
            return false;
        }
    }
    else if(mapperId == 2)
    {
        mapper = new Mapper2(PRG_ROM,CHR_ROM);
    }
    else
    {
        return false;
    }

    if(header.v1.Flags6 & 0x01)
    {
        nametableMirroring = VerticalMirroring;
        printf("\tMirroring vertical\n");
    }
    else
    {
        nametableMirroring = HorizontalMirroring;
        printf("\tMirroring horizontal\n");
    }

    cout << endl;
    return true;
}


void Cartridge::CPU_Write(Byte value, Address address)
{
    mapper->CPU_Write(value,address);
}

Byte Cartridge::CPU_Read(Address address)
{
    return mapper->CPU_Read(address);
}

Byte Cartridge::PPU_Read(Address address)
{
    return  mapper->PPU_Read(address);
}
