#include "ppu.h"
#include "constants.h"
#include <cstring>
#include <unistd.h>
#include "compilationSettings.h"

PPU::PPU()
{
    PPUSTATUS.VBlank = 0;
    //PPUSTATUS.value =0xA0;
    PPUSTATUS.value =0x00;
    PPUCTRL.value = 0;
    OAMADDR = 0;

    currentSpriteNumber = 0;
    spriteEvaluationIndex = 0;
    spriteFetchingIndex = 0;

    internalBuffer = 0;

    cycle = 0;
    scanline = 0;

    patternIndex = 0;

    pixelOffsetX = 0;

    patternLSB = 0;

    NMI = false;
    frameComplete = false;
    oddFrame = false;
    spriteZeroLoaded = false;
    spriteZeroPrepared = false;
}

void PPU::connectCartridge(Cartridge *cartridge)
{
    this->cartridge = cartridge;
}

void PPU::disconnectCartridge()
{
    this->cartridge = NULL;
}

void PPU::loadPixelBuffer(Byte *pixels)
{
    this->pixels = pixels;
}

void PPU::unloadPixelBuffer()
{
    this->pixels = NULL;
}

void PPU::executeCycle()
{
    //Visible frame render (scanlines 0-239)
    if(scanline <= 239)
    {
        //The sprite counter for the current scanline is updated, next scanline's counter and sprite evaluation index are restored to 0
        if(cycle == 0)
        {
            currentSpriteNumber = nextScanlineSpriteNumber;
            nextScanlineSpriteNumber = 0;
            spriteZeroLoaded = spriteZeroPrepared;
            spriteZeroPrepared = false;
            spriteEvaluationIndex = 0;
            spriteFetchingIndex = 0;
        }


        //If we are on a part of the screen that is not blank (1-256), we load the pixel into the buffer
        if(cycle <= 256 && cycle >=1 && PPUMASK.renderBackground)
        {
            if(currentSpriteNumber == 0 || scanline == 0 || !PPUMASK.renderSprites)
            {
                loadBackgroundPixel();
            }
            else
            {
                Byte spriteColorId = 0;
                int spriteIndex = 0;
                int i = 0;

                while (i < currentSpriteNumber)
                {
                    {
                        if(cycle > spriteXposition[i] && cycle <= (spriteXposition[i] + 8))
                        {
                            Byte spriteColorLSB;
                            Byte spriteColorMSB;
                            if(spriteAttribute[i] & 0x40)
                            {
                                //Sprite is flipped horizontally
                                spriteColorLSB = getSpriteColorLSB(i,true);
                                spriteColorMSB = getSpriteColorMSB(i,true);
                            }
                            else
                            {
                                //Sprite is NOT flipped horizontally
                                spriteColorLSB = getSpriteColorLSB(i,false);
                                spriteColorMSB = getSpriteColorMSB(i,false);
                            }

                            if(spriteColorId == 0)
                            {
                                //Try to find the first sprite in secondary OAM that is not transparent (colorId != 0)
                                spriteColorId = Utils::joinBits(spriteColorMSB,spriteColorLSB);
                                spriteIndex = i;
                            }
                        }
                        i++;
                    }
                }

                Byte offset = 15 - pixelOffsetX;
                backgroundColorId = getBackgroundColor(offset);

                Byte colorIndex = 0;

                if(spriteColorId == 0 || (backgroundColorId && (spriteAttribute[spriteIndex] & 0x20)))
                {
                    //The background pixel is selected
                    backgroundPaletteIndex = getBackgroundPalette(offset);
                    colorIndex = getNESPaletteColor(backgroundPaletteIndex,backgroundColorId,false);
                }
                else
                {
                    //The sprite pixel is selected
                    Byte spritePaletteIndex = spriteAttribute[spriteIndex] & 0x3;
                    colorIndex = getNESPaletteColor(spritePaletteIndex,spriteColorId,true);
                }

                //Load the selected pixel
                loadPixel(colorIndex);

                //Check for sprite zero hit (If the sprite zero is loaded it will be on index 0 of the secondary OAM)
                if(spriteIndex == 0 && spriteZeroLoaded)
                {
                    //Sprite Zero Hit Flag is set when an opaque background pixel overlaps an opaque sprite pixel
                    if(spriteColorId && backgroundColorId)
                    {
                        PPUSTATUS.spriteZeroHit = 1;
                    }
                }
            }

            //During the last 64 cycles of the visible scanline, the sprite evaluation for the next scanline is performed
            if(cycle >= 193 && PPUMASK.renderSprites)
            {
                spriteEvaluation();
            }
        }

        if((cycle <= 256 && cycle >= 1) || (cycle >= 328 && cycle <=336))
        {
            //Each cycle, the shift registers are shifted
            shiftBackgroundRegisters();
        }

        if((cycle <= 256 && cycle >= 2) || (cycle >= 322 && cycle <=336))
        {
            //This sequence is repeated every 8 cycles to load all the data for the next two tiles
            backgroundFetching();
        }

        //At cycle 256 of every scanline, the Y component of the current VRAM address is incremented
        if(cycle == 256 && renderEnabled())
        {
            incrementVRAM_AddressY();
        }

        //At cycle 257 of every scanline, the X component of the current VRAM address is updated with the contents of the temporal VRAM address
        if(cycle == 257 && renderEnabled())
        {
            restoreVRAM_AddressX();
        }

        if(cycle >= 259 && cycle <= 320)
        {
            if(PPUMASK.renderSprites)
                spriteFetching();
        }
    }
    //Pre-render scanline
    else if(scanline == 261)
    {
        //At the start of the pre-render scanline, the Vertical Blank flag is reset. Sprite Zero Hit and Sprite Overflow flags are cleared too.
        if(cycle == 1)
        {
            PPUSTATUS.VBlank = 0;
            PPUSTATUS.spriteZeroHit = 0;
            PPUSTATUS.spriteOverflow = 0;
        }

        if((cycle >= 328 && cycle <=336))
        {
            //During the last cycles of the scanline, the background registers are shifted
            shiftBackgroundRegisters();
        }

        if((cycle <= 256 && cycle >= 2) || (cycle >= 322 && cycle <=336))
        {
            //This sequence is repeated every 8 cycles to load all the data for the next two tiles
            backgroundFetching();
        }

        //At cycle 256 of every scanline, the Y component of the current VRAM address is incremented
        if(cycle == 256 && renderEnabled())
        {
            incrementVRAM_AddressY();
        }

        //At cycle 257 of every scanline, the X component of the current VRAM address is updated with the contents of the temporal VRAM address
        if(cycle == 257 && renderEnabled())
        {
            restoreVRAM_AddressX();
        }

        //During cycles 280-304 of the pre-render scanline, the X component of the VRAM address is repeatedly updated with the contents of the temporal VRAM address
        if(cycle >= 280 && cycle <= 304 && renderEnabled())
        {
            restoreVRAM_AddressY();
        }
    }

    //From scanline 241 onwards, the Vertical Blank flag is set
    if(cycle == 1 && scanline == 241)
    {
        PPUSTATUS.VBlank = 1;
        if(PPUCTRL.generateNMI)
        {
            NMI = true;
        }
    }

    //At the end of the visible part of scanline 239, the frame is complete
    if(cycle == 257 && scanline == 239)
    {
        frameComplete = true;
    }

    cycle++;
    if(cycle == 341)
    {
        cycle = 0;
        scanline++;
    }

    if(scanline == 262)
    {
        scanline = 0;
    }

    //The first cycle of the next frame is skipped if it's an odd frame
    oddFrame = !oddFrame;
    if(oddFrame && cycle == 340 && scanline == 261 && renderEnabled())
    {
        cycle = 0;
        scanline = 0;
    }

}

//CPU Access to PPU Registers
Byte PPU::cpuRead(Address address)
{
    switch (address) {
    case RegAddress::PPUCTRL:
    {
        break;
    }
    case RegAddress::PPUMASK:
    {
        break;
    }
    case RegAddress::PPUSTATUS:
    {
        Byte result = (PPUSTATUS.value & 0xE0) | (internalBuffer & 0x1F);
        PPUSTATUS.VBlank = 0;
        addressLatch = 0;
        return result;
    }
    case RegAddress::OAMADDR:
    {
        break;
    }
    case RegAddress::OAMDATA:
    {
        return OAM[0][OAMADDR];
        break;
    }
    case RegAddress::PPUSCROLL:
    {
        break;
    }
    case RegAddress::PPUADDR:
    {
        break;
    }
    case RegAddress::PPUDATA:
    {
        //The data is read from the ppu memory map
        Byte readData = memoryRead(VRAMAdress.value);
        Byte result;

        //If the address is within the palette range, we return the read value directly. Otherwise, we return the value of the previous read stored on the internal buffer.
        if((VRAMAdress.value & 0x3FFF) >= 0x3F00)
            result = readData;
        else
            result = internalBuffer;

        //The internal buffer is always updated with the read data
        internalBuffer = readData;

        //The current VRAM address is incremented by the amount specified in bit 2 of PPUCTRL ('0' --> increment by 1, '1' --> increment by 32)
        if(PPUCTRL.VRAMincrement)
            VRAMAdress.value += 32;
        else
            VRAMAdress.value++;

        return result;
    }

    }
    return 0x00;
}

void PPU::cpuWrite(Byte value, Address address)
{
    switch (address) {
    case RegAddress::PPUCTRL:
    {
        PPUCTRL.value = value;
        tempVRAMAdress.nametableX = PPUCTRL.nametableX;
        tempVRAMAdress.nametableY = PPUCTRL.nametableY;
        break;
    }
    case RegAddress::PPUMASK:
    {
        PPUMASK.value = value;
        break;
    }
    case RegAddress::PPUSTATUS:
    {
        break;
    }
    case RegAddress::OAMADDR:
    {
        OAMADDR = value;
        break;
    }
    case RegAddress::OAMDATA:
    {
        OAM[0][OAMADDR] = value;
        OAMADDR++;
        break;
    }
    case RegAddress::PPUSCROLL:
    {
        if(addressLatch == 0)
        {
            tempVRAMAdress.tileOffsetX = value >> 3;
            pixelOffsetX = value & 0x7;
            addressLatch = 1;
        }
        else //if(addressLatch == 1)
        {
            tempVRAMAdress.pixelOffsetY = value & 0x7;
            tempVRAMAdress.tileOffsetY = value >> 3;
            addressLatch = 0;
        }
        break;
    }
    case RegAddress::PPUADDR:
    {
        if(addressLatch == 0)
        {
            tempVRAMAdress.HByte = value & 0x3F;
            addressLatch = 1;
        }
        else //if(addressLatch == 1)
        {
            tempVRAMAdress.LByte = value;
            VRAMAdress.value = tempVRAMAdress.value;
            addressLatch = 0;
        }
        break;
    }
    case RegAddress::PPUDATA:
    {
        //Write the value to the current VRAM Address
        memoryWrite(value,VRAMAdress.value);

        //The current VRAM address is incremented by the amount specified in bit 2 of PPUCTRL ('0' --> increment by 1, '1' --> increment by 32)
        if(PPUCTRL.VRAMincrement)
            VRAMAdress.value += 32;
        else
            VRAMAdress.value++;
        break;
    }
    default:
        break;
    }
}

QString PPU::stringPPUState()
{
    return QString("PPU %1,%2").arg(cycle,3,10,QLatin1Char(' ')).arg(scanline,3,10,QLatin1Char(' '));
}

//Memory
Byte PPU::memoryRead(Address address)
{
    Address realAddress = address;

    address = address & 0x3FFF;

    //Pattern tables in CHR_ROM
    if(address <= 0x1FFF)
    {
        //printf("Lectura [%04X] --> CHR_ROM[%04X]\n", realAddress, address);
        return cartridge->PPU_Read(address);
    }

    //Nametables and their mirrors
    else if(address >= 0x2000 && address <= 0x3EFF)
    {
        if(cartridge->getMirroringType() == HorizontalMirroring)
        {
#ifdef PRINTLOG
            //printf("Lectura [%04X] -->  Nametable[%d][%04X]\n", realAddress, (address >> 11) & 1, address & 0x03FF);
#endif
            return NameTables[(address >> 11) & 1][address & 0x3FF];
        }
        else if(cartridge->getMirroringType() == VerticalMirroring)
        {
#ifdef PRINTLOG
            //printf("Lectura [%04X] -->  Nametable[%d][%04X]\n", realAddress, (address >> 10) & 1, address & 0x03FF);
#endif
            return NameTables[(address >> 10) & 1][address & 0x3FF];
        }
    }

    //Pallette Indexes and their mirrors
    else if(address >= 0x3F00 && address <= 0x3FFF)
    {
#ifdef PRINTLOG
        //printf("Lectura [%04X] --> Palette Index [%04X]\n", realAddress, address & 0x1F);
#endif
        address = address & 0x1F;
        if(address == 0x10)
            return PaletteRAMIndexes[0x00];
        else if(address == 0x14)
            return PaletteRAMIndexes[0x04];
        else if(address == 0x18)
            return PaletteRAMIndexes[0x08];
        else if(address == 0x1C)
            return PaletteRAMIndexes[0x0C];
        else
            return PaletteRAMIndexes[address];
    }

    return 0x00;
}

void PPU::memoryWrite(Byte value, Address address)
{
    Address realAddress = address;

    address = address & 0x3FFF;

    //Pattern tables in CHR_ROM
    if(address <= 0x1FFF)
    {
        //printf("Escritura [%04X] --> CHR_ROM[%04X] ||| ATENCION: Se ha intentado escribir en CHR_ROM\n", realAddress, address);
    }

    //Nametables and their mirrors
    else if(address >= 0x2000 && address <= 0x3EFF)
    {
        if(cartridge->getMirroringType() == HorizontalMirroring)
        {
#ifdef PRINTLOG
            //printf("Escritura [%04X] --> Nametable[%d][%04X]\n", realAddress, (address >> 11) & 1, address & 0x3FF);
#endif
            NameTables[(address >> 11) & 1][address & 0x3FF] = value;
        }
        else if(cartridge->getMirroringType() == VerticalMirroring)
        {
#ifdef PRINTLOG
            //printf("Escritura [%04X] --> Nametable[%d][%04X]\n", realAddress, (address >> 10) & 1, address & 0x3FF);
#endif
            NameTables[(address >> 10) & 1][address & 0x3FF] = value;
        }
    }

    //Pallette Indexes and their mirrors
    else if(address >= 0x3F00 && address <= 0x3FFF)
    {
#ifdef PRINTLOG
        //printf("Escritura [%04X] --> Palette Index [%04X]\n", realAddress, address & 0x1F);
#endif
        address = address & 0x1F;
        if(address == 0x10)
            PaletteRAMIndexes[0x00] = value;
        else if(address == 0x14)
            PaletteRAMIndexes[0x04] = value;
        else if(address == 0x18)
            PaletteRAMIndexes[0x08] = value;
        else if(address == 0x1C)
            PaletteRAMIndexes[0x0C] = value;
        else
            PaletteRAMIndexes[address] = value;
    }
}

//Scrolling
void PPU::incrementVRAM_AddressX()
{
    if(VRAMAdress.tileOffsetX == 31)
    {
        VRAMAdress.tileOffsetX = 0;
        VRAMAdress.nametableX = ~VRAMAdress.nametableX;
    }
    else
    {
        VRAMAdress.tileOffsetX++;
    }
}

void PPU::incrementVRAM_AddressY()
{
    if(VRAMAdress.pixelOffsetY < 7)
    {
        VRAMAdress.pixelOffsetY++;
    }
    else
    {
        VRAMAdress.pixelOffsetY = 0;
        if(VRAMAdress.tileOffsetY == 29)
        {
            VRAMAdress.tileOffsetY = 0;
            VRAMAdress.nametableY = ~VRAMAdress.nametableY;
        }
        else if(VRAMAdress.tileOffsetY == 31)
        {
            VRAMAdress.tileOffsetY = 0;
            //No nametable switch
        }
        else
        {
            VRAMAdress.tileOffsetY++;
        }

    }
}

void PPU::restoreVRAM_AddressX()
{
    VRAMAdress.tileOffsetX = tempVRAMAdress.tileOffsetX;
    VRAMAdress.nametableX = tempVRAMAdress.nametableX;
}

void PPU::restoreVRAM_AddressY()
{
    VRAMAdress.tileOffsetY = tempVRAMAdress.tileOffsetY;
    VRAMAdress.nametableY = tempVRAMAdress.nametableY;
    VRAMAdress.pixelOffsetY = tempVRAMAdress.pixelOffsetY;
}

//Render
void PPU::loadPixel(Byte colorIndex)
{
    memcpy(&pixels[(scanline * PICTURE_WIDTH + cycle-1) * PIXEL_SIZE],&NESPallette[colorIndex][0],PIXEL_SIZE);
}

Byte PPU::getNESPaletteColor(int paletteIndex, Byte colorOffset, bool spritePalette)
{

    if(colorOffset == 0)
    {
        return memoryRead(0x3F00);
    }
    else
    {

        Byte index = paletteIndex*4 + colorOffset;

        if(spritePalette)
        {
            return memoryRead(0x3F00 + index + 0x10);
        }
        else
        {
            return memoryRead(0x3F00 + index);
        }
    }
}

//Background
void PPU::backgroundFetching()
{
    switch ((cycle-1)%8)
    {
    case 1:
        //Load pattern index from Nametable
        patternIndex = memoryRead(0x2000 | (VRAMAdress.value & 0x0FFF));
        break;

    case 3:
        //Load byte from Attribute table
        tileAttribute = memoryRead(0x23C0 | (VRAMAdress.value & 0xC00) | ((VRAMAdress.tileOffsetY >> 2) << 3) | (VRAMAdress.tileOffsetX >> 2));

        //Only keep the information for the quadrant that the current pixel belongs to
        if(VRAMAdress.tileOffsetY & 0x02)
            tileAttribute >>= 4;
        if(VRAMAdress.tileOffsetX & 0x02)
            tileAttribute >>= 2;
        tileAttribute &= 0x03;
        break;

    case 5:
        //Load pattern low byte from Pattern Table
        patternLSB = memoryRead((PPUCTRL.backgroundAddress << 12) + patternIndex * PTRN_SIZE + VRAMAdress.pixelOffsetY);
        break;

    case 7:
        //Load pattern high byte from Pattern Table
        patternMSB = memoryRead((PPUCTRL.backgroundAddress << 12) + patternIndex * PTRN_SIZE + VRAMAdress.pixelOffsetY + (PTRN_SIZE / 2));

        if(renderEnabled())
        {
            //At this point, we've got all the information for the next tile, so we load it in the LSB of the shift registers
            loadBackgroundShiftRegisters();

            //Increment x component of VRAM
            incrementVRAM_AddressX();
        }
        break;
    }
}

void PPU::loadBackgroundShiftRegisters()
{
    //Load data for the next pattern on the LSB of the shift registers
    shft_PatternLSB.value &= 0xFF00;
    shft_PatternMSB.value &= 0xFF00;
    shft_PatternLSB.value |= patternLSB;
    shft_PatternMSB.value |= patternMSB;

    //Load the (same) palette index on the LSB of the shift registers
    if(tileAttribute & 0b01)
        shft_PaletteLow.value |= 0xFF;
    else
        shft_PaletteLow.value |= 0x00;

    if(tileAttribute & 0b10)
        shft_PaletteHigh.value |= 0xFF;
    else
        shft_PaletteHigh.value |= 0x00;
}

Byte PPU::getBackgroundPalette(Byte offset)
{
    Byte LSB = (shft_PaletteLow.value >> offset) & 0x1;
    Byte MSB = (shft_PaletteHigh.value >> offset) & 0x1;

    return Utils::joinBits(MSB,LSB);
}

Byte PPU::getBackgroundColor(Byte offset)
{
    Byte LSB = (shft_PatternLSB.value >> offset) & 0x1;
    Byte MSB = (shft_PatternMSB.value >> offset) & 0x1;

    return Utils::joinBits(MSB,LSB);
}

void PPU::loadBackgroundPixel()
{
    Byte offset = 15 - pixelOffsetX;

    backgroundPaletteIndex = getBackgroundPalette(offset);
    backgroundColorId = getBackgroundColor(offset);

    Byte colorIndex = getNESPaletteColor(backgroundPaletteIndex,backgroundColorId,false);
    loadPixel(colorIndex);
}

void PPU::shiftBackgroundRegisters()
{
    if(PPUMASK.renderBackground)
    {
        shft_PatternLSB.shiftLeft();
        shft_PatternMSB.shiftLeft();
        shft_PaletteLow.shiftLeft();
        shft_PaletteHigh.shiftLeft();
    }
}

//Sprite
void PPU::spriteEvaluation()
{
    if(nextScanlineSpriteNumber < 9 )
    {
        //Check the sprite Y coordinate to see if the scanline is in the range of the sprite
        if((scanline) >= OAM[spriteEvaluationIndex][0] && (scanline) <= (OAM[spriteEvaluationIndex][0] + 7 + 8* PPUCTRL.spriteSize))
        {
            if(nextScanlineSpriteNumber < 8)
            {
                //Transfer OAM entry to secondary OAM
                memcpy(&Secondary_OAM[nextScanlineSpriteNumber][0],&OAM[spriteEvaluationIndex][0],4);

                //Check if the trasnfered entry is the sprite zero
                if(spriteEvaluationIndex == 0)
                {
                    //Sprite Zero is prepared (loaded for the next scanline)
                    spriteZeroPrepared = true;
                }

                //Update sprite counter
                nextScanlineSpriteNumber++;
            }
            else
            {
                PPUSTATUS.spriteOverflow = 1;
            }
        }

        spriteEvaluationIndex++;
    }
}

void PPU::spriteFetching()
{
    if(spriteFetchingIndex < nextScanlineSpriteNumber)
    {
        switch ((cycle-1)%8)
        {
        case 2:
            //Load [Attribute byte] of the current sprite from the secondary OAM
            spriteAttribute[spriteFetchingIndex] = Secondary_OAM[spriteFetchingIndex][2];
            break;
        case 3:
        {
            //Load [X position] of the current sprite from the secondary OAM
            spriteXposition[spriteFetchingIndex] = Secondary_OAM[spriteFetchingIndex][3];
            break;
        }
        case 5:
            //Load [Pattern Low Byte] of the current sprite from Pattern Table using the pattern index from the secondary OAM
            if((Secondary_OAM[spriteFetchingIndex][2] & 0x80))
            {
                //Sprite flipped vertically
                int size = 8;
                if(PPUCTRL.spriteSize)
                {
                    //8x16 sprites
                    size = 16;
                }
                shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead((PPUCTRL.spriteAdress << 12) + Secondary_OAM[spriteFetchingIndex][1] * PTRN_SIZE + Secondary_OAM[spriteFetchingIndex][0] + (size-1) - scanline);
            }
            else
            {
                //Sprite NOT flipped
                shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead((PPUCTRL.spriteAdress << 12) + Secondary_OAM[spriteFetchingIndex][1] * PTRN_SIZE + scanline - Secondary_OAM[spriteFetchingIndex][0]);
            }
            break;

        case 7:
            //Load [Pattern High Byte] of the current sprite from Pattern Table using the pattern index from the secondary OAM
            if((Secondary_OAM[spriteFetchingIndex][2] & 0x80))
            {
                //Sprite flipped vertically
                int size = 8;
                if(PPUCTRL.spriteSize)
                {
                    //8x16 sprites
                    size = 16;
                }
                shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead((PPUCTRL.spriteAdress << 12) + Secondary_OAM[spriteFetchingIndex][1] * PTRN_SIZE + Secondary_OAM[spriteFetchingIndex][0] + (size-1) - scanline + (PTRN_SIZE / 2));
            }
            else
            {
                //Sprite NOT flipped
                shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead((PPUCTRL.spriteAdress << 12) + Secondary_OAM[spriteFetchingIndex][1] * PTRN_SIZE + scanline - Secondary_OAM[spriteFetchingIndex][0] + (PTRN_SIZE / 2));
            }

            spriteFetchingIndex++;
            break;
        }
    }
}

Byte PPU::getSpriteColorLSB(int i, bool spriteFlipped)
{
    Byte colorId = 0;
    if(spriteFlipped)
    {
        colorId = shft_SpritePatternLSB[i].value & 0x01;
        shft_SpritePatternLSB[i].shiftRight();
    }
    else
    {
        colorId = (shft_SpritePatternLSB[i].value & 0x80) >> 7;
        shft_SpritePatternLSB[i].shiftLeft();
    }
    return colorId;
}

Byte PPU::getSpriteColorMSB(int i, bool spriteFlipped)
{
    Byte colorId = 0;
    if(spriteFlipped)
    {
        colorId = shft_SpritePatternMSB[i].value & 0x01;
        shft_SpritePatternMSB[i].shiftRight();
    }
    else
    {
        colorId = (shft_SpritePatternMSB[i].value & 0x80) >> 7;
        shft_SpritePatternMSB[i].shiftLeft();
    }
    return colorId;
}
