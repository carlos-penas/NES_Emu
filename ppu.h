#ifndef PPU_H
#define PPU_H
#include "types.h"
#include "cartridge.h"
#include "utils.h"

using namespace PPURegisters;

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


class PPU
{
public:
    PPU();

    void connectCartridge(Cartridge *cartridge);
    void disconnectCartridge();

    void loadPixelBuffer(Byte * pixels);
    void unloadPixelBuffer();

    uint16_t getCurrentScanline() {return scanline;};
    uint16_t getCurrentCycle(){return cycle;};

    void executeCycle();

    //CPU Access to PPU Registers
    Byte cpuRead(Address address);
    void cpuWrite(Byte value, Address address);

    string stringPPUState();

    bool NMI;
    bool frameComplete;

private:
    //Memory map (16KB)
    Cartridge * cartridge;                  //   8 KB
    Register8 NameTables[2][0x400];         //   2 NameTables of 1KB each
    Register8 PaletteRAMIndexes[0x20];      //  32 Bytes

    //Internal memory
    Register8 OAM[64][4];                   // 256 Bytes
    Register8 Secondary_OAM[8][4];          //  32 Bytes

    //Buffer to render the pixels
    Byte *pixels;

    //Cycles
    uint16_t cycle;
    uint16_t scanline;

    //Flags
    bool oddFrame;
    bool spriteZeroLoaded;
    bool spriteZeroPrepared;

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

    Byte NESPallette [64][4] =
    {
        {84,84,84,255},   {0,30,116,255},   {8,16,144,255},   {48,0,136,255},   {68,0,100,255},   {92,0,48,255},    {84,4,0,255},     {60,24,0,255},    {32,42,0,255},    {8,58,0,255},     {0,64,0,255},     {0,60,0,255},     {0,50,60,255},    {0,0,0,0},        {0,0,0,0},{0,0,0,0},
        {152,150,152,255},{8,76,196,255},   {48,50,236,255},  {92,30,228,255},  {136,20,176,255}, {160,20,100,255}, {152,34,32,255},  {120,60,0,255},   {84,90,0,255},    {40,114,0,255},   {8,124,0,255},    {0,118,40,255},   {0,102,120,255},  {0,0,0,0},        {0,0,0,0},{0,0,0,0},
        {236,238,236,255},{76,154,236,255}, {120,124,236,255},{176,98,236,255}, {228,84,236,255}, {236,88,180,255}, {236,106,100,255},{212,136,32,255}, {160,170,0,255},  {116,196,0,255},  {76,208,32,255},  {56,204,108,255}, {56,180,204,255}, {60,60,60,255},   {0,0,0,0},{0,0,0,0},
        {236,238,236,255},{168,204,236,255},{188,188,236,255},{212,178,236,255},{236,174,236,255},{236,174,212,255},{236,180,176,255},{228,196,144,255},{204,210,120,255},{180,122,120,255},{168,226,144,255},{152,226,180,255},{160,214,228,255},{160,162,160,255},{0,0,0,0},{0,0,0,0}
    };

    //Background Rendering
    Byte patternIndex;
    Byte tileAttribute;
    Byte patternLSB;
    Byte patternMSB;

    ShiftRegister16 shft_PatternLSB;
    ShiftRegister16 shft_PatternMSB;
    ShiftRegister16 shft_PaletteLow;
    ShiftRegister16 shft_PaletteHigh;

    Byte backgroundColorId;
    Byte backgroundPaletteIndex;

    //Sprite Rendering
    ShiftRegister8 shft_SpritePatternLSB[8];
    ShiftRegister8 shft_SpritePatternMSB[8];

    Register8 spriteXposition[8];
    Register8 spriteAttribute[8];

    Byte currentSpriteNumber;
    Byte nextScanlineSpriteNumber;
    Byte spriteEvaluationIndex;
    Byte spriteFetchingIndex;

    //Memory
    Byte memoryRead(Address address);
    void memoryWrite(Byte value, Address address);

    //Scrolling
    void incrementVRAM_AddressX();
    void incrementVRAM_AddressY();
    void restoreVRAM_AddressX();
    void restoreVRAM_AddressY();

    //Render
    void loadPixel(Byte colorIndex);
    bool renderEnabled(){return PPUMASK.renderBackground || PPUMASK.renderSprites;};
    Byte getNESPaletteColor(int paletteIndex, Byte backgroundColorId, bool spritePalette);

    //Background
    void backgroundFetching();
    void loadBackgroundShiftRegisters();
    Byte getBackgroundPalette(Byte offset);
    Byte getBackgroundColor(Byte offset);
    void loadBackgroundPixel();
    void shiftBackgroundRegisters();

    //Sprite
    void spriteEvaluation();
    void spriteFetching();
    Byte getSpriteColorLSB(int i, bool spriteFlipped);
    Byte getSpriteColorMSB(int i, bool spriteFlipped);
};

#endif // PPU_H
