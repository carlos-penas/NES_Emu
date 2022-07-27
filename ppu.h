#ifndef PPU_H
#define PPU_H
#include "types.h"
#include "cartridge.h"
#include "bus.h"

//SFML
#include "Graphics.hpp"


class PPU
{
public:
    PPU(Bus *bus);

    void connectCartridge(Cartridge *cartridge);

    void drawFrame();
private:
    //Memory map (16KB)
    Cartridge * cartridge;                  //  8 KB
    Register8 NameTable0[0x400];            //  1 KB
    Register8 NameTable1[0x400];            //  1 KB
    Register8 NameTable2[0x400];            //  1 KB
    Register8 NameTable3[0x400];            //  1 KB
    Register8 PaletteRAMIndexes[0x20];      // 32 Bytes

    //CPU bus
    Bus *bus;

    //SFML
    sf::RenderWindow window;



};

#endif // PPU_H
