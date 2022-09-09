#ifndef MAPPER2_H
#define MAPPER2_H
#include "mapper.h"

class Mapper2 : public Mapper
{
public:
    Mapper2(vector<Register8> PRG_ROM, vector<Register8> CHR_ROM);

    void CPU_Write(Byte value, Address address) override;
    Byte CPU_Read(Address address) override;

    Byte* PPU_GetPattern(uint16_t index) override;
    Byte PPU_Read(Address address) override;
};

#endif // MAPPER2_H
