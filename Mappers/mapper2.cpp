#include "mapper2.h"

Mapper2::Mapper2(std::vector<Register8> PRG_ROM, std::vector<Register8> CHR_ROM)
    : Mapper(PRG_ROM,CHR_ROM)
{

}

void Mapper2::CPU_Write(Byte value, Address address)
{
    //TO BE IMPLEMENTED
}

Byte Mapper2::CPU_Read(Address address)
{
    //TO BE IMPLEMENTED
    return 0;
}


Byte Mapper2::PPU_Read(Address address)
{
    //TO BE IMPLEMENTED
    return 0;
}

