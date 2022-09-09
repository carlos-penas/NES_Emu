#ifndef PPU_H
#define PPU_H
#include "types.h"
#include "cartridge.h"
#include "bus.h"

//SFML
#include "Graphics.hpp"
typedef uint16_t  Pattern_Index;

class PPU
{
public:
    PPU(Bus *bus);

    void connectCartridge(Cartridge *cartridge);

    void drawFrame();

    void drawPatternTable();
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
    Byte color1 [4] = {0,0,0,0};
    Byte color2 [4] = {247, 2, 149,255};
    Byte color3 [4] = {65, 242, 216,255};
    Byte color4 [4] = {120, 10, 10,255};

//    Byte color1 [4] = {0,0,0,0};
//    Byte color2 [4] = {64, 64, 64,255};
//    Byte color3 [4] = {128, 128, 128,255};
//    Byte color4 [4] = {192, 192, 192,255};

    void loadPattern(int tableRow, int tableCol, Byte *pixels);
    void loadPatternRow(Byte *pixels, int ptrn_row, int patternIndex);
    int getPatternIndex(int tableRow, int tableCol);



};

#endif // PPU_H
