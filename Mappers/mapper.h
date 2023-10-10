#ifndef MAPPER_H
#define MAPPER_H
#include <vector>

#include "../types.h"

using namespace std;

class Mapper
{
public:
    Mapper(vector<Register8> PRG_ROM);
    Mapper(vector<Register8> PRG_ROM, vector<Register8> CHR_ROM);

    virtual ~Mapper() = default;

    virtual void CPU_Write(Byte value, Address address) = 0;
    virtual Byte CPU_Read(Address address) = 0;

    virtual Byte PPU_Read(Address address) = 0;

protected:
    vector<Register8> PRG_ROM;
    vector<Register8> CHR_ROM;
};

#endif // MAPPER_H
