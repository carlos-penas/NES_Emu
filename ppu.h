#ifndef PPU_H
#define PPU_H
#include "types.h"
#include "cartridge.h"
#include "utils.h"

//SFML
#include "Graphics.hpp"

enum RegAddress{
    PPUCTRL   = 0x2000,
    PPUMASK   = 0x2001,
    PPUSTATUS = 0x2002,
    OAMADDR   = 0x2003,
    OAMDATA   = 0x2004,
    PPUSCROLL = 0x2005,
    PPUADDR   = 0x2006,
    PPUDATA   = 0x2007,
};

struct AddressRegister{
    Byte latch;
    Register16 address;

    void setLatch() {latch = 1;}
    void resetLatch() {latch = 0;}

    bool latch_isSet() {return latch;}

    void setLowByte(Byte value) {address = Utils::joinBytes(Utils::highByte(address),value);}
    void setHighByte(Byte value) {address = Utils::joinBytes(value,Utils::lowByte(address));}
};

class PPU
{
public:
    PPU();

    void connectCartridge(Cartridge *cartridge);
    void disconnectCartridge();

    void executeCycle();

    void drawFrame();

    void drawPatternTable();

    void drawNameTable();

    //When cpu tries to read from PPU registers
    void cpuWrite(Byte value, Address address);
    Byte cpuRead(Address address);

    void OAM_DMA_Transfer(Byte *cpuAddress);

    QString stringPPUState();

    bool NMI;

private:
    //Memory map (16KB)
    Cartridge * cartridge;                  //   8 KB
    Register8 NameTables[4][0x400];         //   4 NameTables of 1KB each
    Register8 PaletteRAMIndexes[0x20];      //  32 Bytes
    Register8 OAM[0x100];                   // 256 Bytes

    //Cycles
    uint16_t cycle;
    uint16_t scanline;

    //SFML
    sf::RenderWindow window;
    Byte color1 [4] = {0,0,0,0};
    Byte color2 [4] = {247, 2, 149,255};
    Byte color4 [4] = {65, 242, 216,255};
    Byte color3 [4] = {120, 10, 10,255};

//    Byte color1 [4] = {0,0,0,0};
//    Byte color2 [4] = {64, 64, 64,255};
//    Byte color3 [4] = {128, 128, 128,255};
//    Byte color4 [4] = {192, 192, 192,255};

    //Byte NESPallette [256];
    Byte NESPallette [64][4] =
    {
        {84,84,84,255},   {0,30,116,255},   {8,16,144,255},   {48,0,136,255},   {68,0,100,255},   {92,0,48,255},    {84,4,0,255},     {60,24,0,255},    {32,42,0,255},    {8,58,0,255},     {0,64,0,255},     {0,60,0,255},     {0,50,60,255},    {0,0,0,0},        {0,0,0,0},{0,0,0,0},
        {152,150,152,255},{8,76,196,255},   {48,50,236,255},  {92,30,228,255},  {136,20,176,255}, {160,20,100,255}, {152,34,32,255},  {120,60,0,255},   {84,90,0,255},    {40,114,0,255},   {8,124,0,255},    {0,118,40,255},   {0,102,120,255},  {0,0,0,0},        {0,0,0,0},{0,0,0,0},
        {236,238,236,255},{76,154,236,255}, {120,124,236,255},{176,98,236,255}, {228,84,236,255}, {236,88,180,255}, {236,106,100,255},{212,136,32,255}, {160,170,0,255},  {116,196,0,255},  {76,208,32,255},  {56,204,108,255}, {56,180,204,255}, {60,60,60,255},   {0,0,0,0},{0,0,0,0},
        {236,238,236,255},{168,204,236,255},{188,188,236,255},{212,178,236,255},{236,174,236,255},{236,174,212,255},{236,180,176,255},{228,196,144,255},{204,210,120,255},{180,122,120,255},{168,226,144,255},{152,226,180,255},{160,214,228,255},{160,162,160,255},{0,0,0,0},{0,0,0,0}
    };

    Byte getPaletteIndex(int blockIndex, int blockAttribute);

    void loadPattern(int tableRow, int tableCol, Byte *pixels);
    void loadPatternRow(Byte *pixels, int ptrn_row, int patternIndex, int paletteIndex);
    int getPatternIndex(int tableRow, int tableCol);
    Byte getColor(int paletteIndex, Byte colorOffset);

    //Memory
    void memoryWrite(Byte value, Address address);
    Byte memoryRead(Address address);

    //Registers
    Register8 PPUCTRL;
    Register8 PPUSTATUS;
    Register8 PPUSCROLL;
    Register8 PPUMASK;
    AddressRegister PPUADDR;
    Register8 OAMADDR;
    Register8 OAMDATA;

    Register8 internalBuffer;

    bool PPUCTRL_VRAM_increment();
    bool PPUCTRL_NMI();

    void PPUSTATUS_SetVBlank(bool set);

};

#endif // PPU_H
