#include "mapper.h"
#include <cstring>

Mapper::Mapper(vector<Register8> PRG_ROM)
{
    this->PRG_ROM = PRG_ROM;
}

Mapper::Mapper(vector<Register8> PRG_ROM, vector<Register8> CHR_ROM)
{
    this->PRG_ROM = PRG_ROM;
    this->CHR_ROM = CHR_ROM;
}
