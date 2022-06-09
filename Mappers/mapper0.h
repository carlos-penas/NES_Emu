#ifndef MAPPER0_H
#define MAPPER0_H
#include "mapper.h"

class Mapper0 : public Mapper
{
public:
    Mapper0(vector<Register8> PRG_ROM, uint16_t PRG_RAM_Size);

    void CPU_Write(Byte value, Address address) override;
    Byte CPU_Read(Address address) override;

private:
    vector<Register8> PRG_RAM;
};

#endif // MAPPER0_H
