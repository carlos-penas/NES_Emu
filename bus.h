#ifndef BUS_H
#define BUS_H
#include "types.h"
#include "cartridge.h"
#include "ppu.h"

class Bus
{
public:
    Bus();

    void connectCartridge(Cartridge *cartridge);
    void disconnectCartridge();
    void connectPPU(PPU *ppu);
    void disconnectPPU();

    void Write(Byte value, Address address);
    Byte Read(Address address);

private:
    //Memory Map (64KB)
    Register8 RAM[0x800];           //  2 KB
    Register8 APU_IO[0x18];         // 24 B
    Register8 APU_Test[8];          //  8 B
    Cartridge *cartridge;           // 49 KB

    PPU *ppu;

    void OAM_DMA_Transfer(Byte addressH);
};

#endif // BUS_H
