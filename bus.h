#ifndef BUS_H
#define BUS_H
#include "types.h"
#include "cartridge.h"
class Bus
{
public:
    Bus();

    void connectCartridge(Cartridge *cartridge);

    void Write(Byte value, Address address);
    Byte Read(Address address);


private:
    //Memory Map (64KB)
    Register8 RAM[0x800];           //  2 KB
    Register8 PPU_Register[8];      //  8 B
    Register8 APU_IO[0x18];         // 24 B
    Register8 APU_Test[8];          //  8 B
    Cartridge *cartridge;           // 49KB
};

#endif // BUS_H
