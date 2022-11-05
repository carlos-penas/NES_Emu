#ifndef PPU_H
#define PPU_H
#include "types.h"
#include "cartridge.h"
#include "utils.h"
#define SHOWGRAPHICS

//SFML
#ifdef SHOWGRAPHICS
#include "Graphics.hpp"
#endif

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

union AddressRegister{
    struct{
        Register16 tileOffsetX : 5;
        Register16 tileOffsetY : 5;
        Register16 nametableX : 1;
        Register16 nametableY : 1;
        Register16 pixelOffsetY : 3;
        Register16 unused : 1;
    };
    Register16  value;

    struct{
        Register8 LByte;
        Register8 HByte;
    };
};

union ControlRegister{
    struct{
        Register8 nametableX : 1;
        Register8 nametableY : 1;
        Register8 VRAMincrement : 1;
        Register8 spriteAdress : 1;
        Register8 backgroundAddress : 1;
        Register8 spriteSize : 1;
        Register8 MstSlvMode : 1;
        Register8 generateNMI : 1;
    };
    Register8 value;
};

union StatusRegister{
    struct{
        Register8 unused : 5;
        Register8 spriteOverflow : 1;
        Register8 spriteZeroHit : 1;
        Register8 VBlank : 1;
    };
    Register8 value;
};

union MaskRegister{
    struct{
        Register8 greyScale : 1;
        Register8 showLeftBackground : 1;
        Register8 showLeftSprites : 1;
        Register8 renderBackground : 1;
        Register8 renderSprites : 1;
        Register8 emphasizeRed : 1;
        Register8 emphasizeGreen : 1;
        Register8 emphasizeBlue : 1;
    };
    Register8 value;
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

    //When cpu tries to access from PPU registers
    void cpuWrite(Byte value, Address address);
    Byte cpuRead(Address address);

    void OAM_DMA_Transfer(Byte value, int index);

    QString stringPPUState();

    bool NMI;

private:
    //Memory map (16KB)
    Cartridge * cartridge;                  //   8 KB
    Register8 NameTables[2][0x400];         //   2 NameTables of 1KB each
    Register8 PaletteRAMIndexes[0x20];      //  32 Bytes
    Register8 OAM[0x100];                   // 256 Bytes

    //Cycles
    uint16_t cycle;
    uint16_t scanline;

    //SFML
#ifdef SHOWGRAPHICS
    sf::RenderWindow window;
#endif
    Byte color1 [4] = {0,0,0,0};
    Byte color2 [4] = {247, 2, 149,255};
    Byte color4 [4] = {65, 242, 216,255};
    Byte color3 [4] = {120, 10, 10,255};


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
    ControlRegister PPUCTRL;
    MaskRegister PPUMASK;
    StatusRegister PPUSTATUS;
    Register8 OAMADDR;
    Register8 OAMDATA;

    //Internal registers operated by PPUADDR and PPUSCROLL
    AddressRegister VRAMAdress;     //(15 bits) --> [v]: Current VRAM address
    AddressRegister tempVRAMAdress; //(15 bits) --> [t]: Temporary VRAM address. Address of the first tile (top left) that appears on the screen
    Register8 pixelOffsetX;         //(3 bits)  --> [x]: Fine X scroll
    Byte addressLatch;              //(1 bit)   --> [w]: Address latch shared by PPUADDR and PPUSCROLL (Conmutador)

    Register8 internalBuffer;

};

#endif // PPU_H
