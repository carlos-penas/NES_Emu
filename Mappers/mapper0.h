#ifndef MAPPER0_H
#define MAPPER0_H
#include "mapper.h"

class Mapper0 : public Mapper
{
public:
    Mapper0(std::vector<Register8> PRG_ROM, uint16_t PRG_RAM_Size, std::vector<Register8> CHR_ROM);

    void CPU_Write(Byte value, Address address) override;
    Byte CPU_Read(Address address) override;

    Byte PPU_Read(Address address) override;

private:
    std::vector<Register8> PRG_RAM;
};

#endif // MAPPER0_H
