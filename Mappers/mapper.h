#ifndef MAPPER_H
#define MAPPER_H
#include <vector>

#include "../types.h"

class Mapper
{
public:
    Mapper(std::vector<Register8> PRG_ROM);
    Mapper(std::vector<Register8> PRG_ROM, std::vector<Register8> CHR_ROM);

    virtual ~Mapper() = default;

    virtual void CPU_Write(Byte value, Address address) = 0;
    virtual Byte CPU_Read(Address address) = 0;

    virtual Byte PPU_Read(Address address) = 0;

protected:
    std::vector<Register8> PRG_ROM;
    std::vector<Register8> CHR_ROM;
};

#endif // MAPPER_H
