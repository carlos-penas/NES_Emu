#include "ppu.h"
#include "constants.h"
#include <cstring>

PPU::PPU()
{
    statusRegister.VBlank = 0;
    //statusRegister.value =0xA0;
    statusRegister.value =0x00;
    controlRegister.value = 0;
    OAMAddressRegister = 0;

    currentSpriteNumber = 0;
    spriteEvaluationIndex = 0;
    spriteFetchingIndex = 0;

    internalBuffer = 0;

    cycle = 0;
    scanline = 0;

    patternIndex = 0;

    XRegister = 0;

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
        if(cycle <= 256 && cycle >=1 && maskRegister.renderBackground)
        {
            //If there are no sprites on the scanline, a background pixel is loaded
            if(currentSpriteNumber == 0 || scanline == 0 || !maskRegister.renderSprites)
            {
                loadBackgroundPixel();
            }
            else
            {
                Byte spriteColorId = 0;
                int spriteIndex = 0;
                int i = 0;

                //Try to find the first sprite in secondary OAM that is not transparent (colorId != 0)
                while (i < currentSpriteNumber)
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
                            spriteColorId = Utils::joinBits(spriteColorMSB,spriteColorLSB);
                            spriteIndex = i;
                        }
                    }
                    i++;
                }

                Byte offset = 15 - XRegister;
                backgroundColorId = getBackgroundColor(offset);

                Byte colorIndex = 0;

                //Check priority and transparency to select the appropriate pixel
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
                        statusRegister.spriteZeroHit = 1;
                    }
                }
            }

            //During the last 64 cycles of the visible scanline, the sprite evaluation for the next scanline is performed
            if(cycle >= 193 && maskRegister.renderSprites)
            {
                spriteEvaluation();
            }
        }

        //Each cycle, the shift registers are shifted
        if((cycle <= 256 && cycle >= 1) || (cycle >= 328 && cycle <=336))
        {
            shiftBackgroundRegisters();
        }

        //The background fetching sequence is repeated every 8 cycles to load all the data for the next two tiles
        if((cycle <= 256 && cycle >= 2) || (cycle >= 322 && cycle <=336))
        {
            backgroundFetching();
        }

        //At cycle 256 of every scanline, the Y component of the current VRAM address is incremented
        if(cycle == 256 && renderEnabled())
        {
            incrementVRAM_AddressY();
        }

        //At cycle 257 of every scanline, the X component of the current VRAM address is restored to match the X component of temporal VRAM address
        if(cycle == 257 && renderEnabled())
        {
            restoreVRAM_AddressX();
        }

        //During cycles (259-320), the sprite fetching sequence is performed
        if(cycle >= 259 && cycle <= 320)
        {
            if(maskRegister.renderSprites)
                spriteFetching();
        }
    }
    //Pre-render scanline
    else if(scanline == 261)
    {
        //At the start of the pre-render scanline, the Vertical Blank flag is reset. Sprite Zero Hit and Sprite Overflow flags are cleared too.
        if(cycle == 1)
        {
            statusRegister.VBlank = 0;
            statusRegister.spriteZeroHit = 0;
            statusRegister.spriteOverflow = 0;
        }

        //During the last cycles of the pre-render scanline, the background registers are shifted
        if((cycle >= 328 && cycle <=336))
        {
            shiftBackgroundRegisters();
        }

        //The background fetching sequence is repeated twice to load all the data for the first two tiles of the next scanline
        if(cycle >= 322 && cycle <=336) // || (cycle >= 2 && cycle <= 256)
        {
            backgroundFetching();
        }

        //At cycle 256 of the pre-render scanline, the Y component of the current VRAM address is incremented
        if(cycle == 256 && renderEnabled())
        {
            incrementVRAM_AddressY();
        }

        //At cycle 257 of the pre-render scanline, the X component of the current VRAM address is restored to match the X component of temporal VRAM address
        if(cycle == 257 && renderEnabled())
        {
            restoreVRAM_AddressX();
        }

        //During cycles 280-304 of the pre-render scanline, the X component of the VRAM address is repeatedly restored to match the Y component of temporal VRAM address
        if(cycle >= 280 && cycle <= 304 && renderEnabled())
        {
            restoreVRAM_AddressY();
        }
    }

    //From scanline 241 onwards, the Vertical Blank flag is set
    if(cycle == 1 && scanline == 241)
    {
        statusRegister.VBlank = 1;
        if(controlRegister.generateNMI)
        {
            NMI = true;
        }
    }

    //At the end of the visible part of scanline 239, the frameComplete flag is set
    if(cycle == 257 && scanline == 239)
    {
        frameComplete = true;
    }

    //The cycle and scanline counters are updated
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
        Byte result = (statusRegister.value & 0xE0) | (internalBuffer & 0x1F);
        statusRegister.VBlank = 0;
        latch = 0;
        return result;
    }
    case RegAddress::OAMADDR:
    {
        break;
    }
    case RegAddress::OAMDATA:
    {
        return OAM[0][OAMAddressRegister];
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
        Byte readData = memoryRead(VRegister.value);
        Byte result;

        //If the address is within the palette range, we return the read value directly. Otherwise, we return the value of the previous read stored on the internal buffer.
        if((VRegister.value & 0x3FFF) >= 0x3F00)
            result = readData;
        else
            result = internalBuffer;

        //The internal buffer is always updated with the read data
        internalBuffer = readData;

        //The current VRAM address is incremented by the amount specified in bit 2 of PPUCTRL ('0' --> increment by 1, '1' --> increment by 32)
        if(controlRegister.VRAMincrement)
            VRegister.value += 32;
        else
            VRegister.value++;

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
        controlRegister.value = value;
        TRegister.nametableX = controlRegister.nametableX;
        TRegister.nametableY = controlRegister.nametableY;
        break;
    }
    case RegAddress::PPUMASK:
    {
        maskRegister.value = value;
        break;
    }
    case RegAddress::PPUSTATUS:
    {
        break;
    }
    case RegAddress::OAMADDR:
    {
        OAMAddressRegister = value;
        break;
    }
    case RegAddress::OAMDATA:
    {
        OAM[0][OAMAddressRegister] = value;
        OAMAddressRegister++;
        break;
    }
    case RegAddress::PPUSCROLL:
    {
        if(latch == 0)
        {
            TRegister.tileOffsetX = value >> 3;
            XRegister = value & 0x7;
            latch = 1;
        }
        else //if(latch == 1)
        {
            TRegister.pixelOffsetY = value & 0x7;
            TRegister.tileOffsetY = value >> 3;
            latch = 0;
        }
        break;
    }
    case RegAddress::PPUADDR:
    {
        if(latch == 0)
        {
            TRegister.HByte = value & 0x3F;
            latch = 1;
        }
        else //if(latch == 1)
        {
            TRegister.LByte = value;
            VRegister.value = TRegister.value;
            latch = 0;
        }
        break;
    }
    case RegAddress::PPUDATA:
    {
        //Write the value to the current VRAM Address
        memoryWrite(value,VRegister.value);

        //The current VRAM address is incremented by the amount specified in bit 2 of PPUCTRL ('0' --> increment by 1, '1' --> increment by 32)
        if(controlRegister.VRAMincrement)
            VRegister.value += 32;
        else
            VRegister.value++;
        break;
    }
    default:
        break;
    }
}

std::string PPU::stringPPUState()
{
    std::stringstream s1,s2;
    s1 << std::setfill(' ') << std::setw(3) << cycle;
    s2 << std::setfill(' ') << std::setw(3) << scanline;

    return "PPU " + s1.str() + "," + s2.str();
}

//Memory
Byte PPU::memoryRead(Address address)
{
#ifdef PRINT_LOG
    Address realAddress = address;
#endif

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
#ifdef PRINT_LOG
            //printf("Lectura [%04X] -->  Nametable[%d][%04X]\n", realAddress, (address >> 11) & 1, address & 0x03FF);
#endif
            return NameTables[(address >> 11) & 1][address & 0x3FF];
        }
        else if(cartridge->getMirroringType() == VerticalMirroring)
        {
#ifdef PRINT_LOG
            //printf("Lectura [%04X] -->  Nametable[%d][%04X]\n", realAddress, (address >> 10) & 1, address & 0x03FF);
#endif
            return NameTables[(address >> 10) & 1][address & 0x3FF];
        }
    }

    //Pallette Indexes and their mirrors
    else if(address >= 0x3F00 && address <= 0x3FFF)
    {
#ifdef PRINT_LOG
        //printf("Lectura [%04X] --> Palette Index [%04X]\n", realAddress, address & 0x1F);
#endif
        address = address & 0x1F;
        if(address == 0x10)
            return VRAMPalettes[0x00];
        else if(address == 0x14)
            return VRAMPalettes[0x04];
        else if(address == 0x18)
            return VRAMPalettes[0x08];
        else if(address == 0x1C)
            return VRAMPalettes[0x0C];
        else
            return VRAMPalettes[address];
    }

    return 0x00;
}

void PPU::memoryWrite(Byte value, Address address)
{
#ifdef PRINT_LOG
    Address realAddress = address;
#endif

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
#ifdef PRINT_LOG
            //printf("Escritura [%04X] --> Nametable[%d][%04X]\n", realAddress, (address >> 11) & 1, address & 0x3FF);
#endif
            NameTables[(address >> 11) & 1][address & 0x3FF] = value;
        }
        else if(cartridge->getMirroringType() == VerticalMirroring)
        {
#ifdef PRINT_LOG
            //printf("Escritura [%04X] --> Nametable[%d][%04X]\n", realAddress, (address >> 10) & 1, address & 0x3FF);
#endif
            NameTables[(address >> 10) & 1][address & 0x3FF] = value;
        }
    }

    //Pallette Indexes and their mirrors
    else if(address >= 0x3F00 && address <= 0x3FFF)
    {
#ifdef PRINT_LOG
        //printf("Escritura [%04X] --> Palette Index [%04X]\n", realAddress, address & 0x1F);
#endif
        address = address & 0x1F;
        if(address == 0x10)
            VRAMPalettes[0x00] = value;
        else if(address == 0x14)
            VRAMPalettes[0x04] = value;
        else if(address == 0x18)
            VRAMPalettes[0x08] = value;
        else if(address == 0x1C)
            VRAMPalettes[0x0C] = value;
        else
            VRAMPalettes[address] = value;
    }
}

//Scrolling
void PPU::incrementVRAM_AddressX()
{
    if(VRegister.tileOffsetX == 31)
    {
        VRegister.tileOffsetX = 0;
        VRegister.nametableX = ~VRegister.nametableX;
    }
    else
    {
        VRegister.tileOffsetX++;
    }
}

void PPU::incrementVRAM_AddressY()
{
    if(VRegister.pixelOffsetY < 7)
    {
        VRegister.pixelOffsetY++;
    }
    else
    {
        VRegister.pixelOffsetY = 0;
        if(VRegister.tileOffsetY == 29)
        {
            VRegister.tileOffsetY = 0;
            VRegister.nametableY = ~VRegister.nametableY;
        }
        else if(VRegister.tileOffsetY == 31)
        {
            VRegister.tileOffsetY = 0;
            //No nametable switch
        }
        else
        {
            VRegister.tileOffsetY++;
        }

    }
}

void PPU::restoreVRAM_AddressX()
{
    VRegister.tileOffsetX = TRegister.tileOffsetX;
    VRegister.nametableX = TRegister.nametableX;
}

void PPU::restoreVRAM_AddressY()
{
    VRegister.tileOffsetY = TRegister.tileOffsetY;
    VRegister.nametableY = TRegister.nametableY;
    VRegister.pixelOffsetY = TRegister.pixelOffsetY;
}

//Render
void PPU::loadPixel(Byte colorIndex)
{
    memcpy(&pixels[(scanline * PICTURE_WIDTH + cycle-1) * PIXEL_SIZE],&systemPallette[colorIndex][0],PIXEL_SIZE);
}

Byte PPU::getNESPaletteColor(int paletteIndex, Byte colorOffset, bool spritePalette)
{
    if(colorOffset == 0)
    {
        //The background color is returned
        return memoryRead(0x3F00);
    }
    else
    {
        Byte index = paletteIndex*4 + colorOffset;

        if(spritePalette)
        {
            //The color is selected from one of the sprite palettes (second 16 bytes)
            return memoryRead(0x3F00 + index + 0x10);
        }
        else
        {
            //The color is selected from one of the background palettes (first 16 bytes)
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
        patternIndex = memoryRead(0x2000 | (VRegister.value & 0x0FFF));
        break;

    case 3:
        //Load byte from Attribute table
        attributeByte = memoryRead(0x23C0 | (VRegister.value & 0xC00) | ((VRegister.tileOffsetY >> 2) << 3) | (VRegister.tileOffsetX >> 2));

        //Only keep the information for the quadrant that the current pixel belongs to
        if(VRegister.tileOffsetY & 0x02)
            attributeByte >>= 4;
        if(VRegister.tileOffsetX & 0x02)
            attributeByte >>= 2;
        attributeByte &= 0x03;
        break;

    case 5:
        //Load pattern low byte from Pattern Table
        patternLSB = memoryRead((controlRegister.backgroundAddress << 12) + patternIndex * PTRN_SIZE + VRegister.pixelOffsetY);
        break;

    case 7:
        //Load pattern high byte from Pattern Table
        patternMSB = memoryRead((controlRegister.backgroundAddress << 12) + patternIndex * PTRN_SIZE + VRegister.pixelOffsetY + (PTRN_SIZE / 2));

        if(renderEnabled())
        {
            //At this point, we've got all the information for the next tile, so we load it in the Low Byte of the shift registers
            loadBackgroundShiftRegisters();

            //Increment x component of VRAM
            incrementVRAM_AddressX();
        }
        break;
    }
}

void PPU::loadBackgroundShiftRegisters()
{
    //Load data for the next pattern on the Low Byte of the shift registers
    shft_PatternLSB.value &= 0xFF00;
    shft_PatternMSB.value &= 0xFF00;
    shft_PatternLSB.value |= patternLSB;
    shft_PatternMSB.value |= patternMSB;

    //Load the (same) palette index on the Low Byte of the shift registers
    if(attributeByte & 0b01)
        shft_PaletteLow.value |= 0xFF;
    else
        shft_PaletteLow.value |= 0x00;

    if(attributeByte & 0b10)
        shft_PaletteHigh.value |= 0xFF;
    else
        shft_PaletteHigh.value |= 0x00;
}

Byte PPU::getBackgroundPalette(Byte offset)
{
    Byte lsb = (shft_PaletteLow.value >> offset) & 0x1;
    Byte msb = (shft_PaletteHigh.value >> offset) & 0x1;

    return Utils::joinBits(msb,lsb);
}

Byte PPU::getBackgroundColor(Byte offset)
{
    Byte lsb = (shft_PatternLSB.value >> offset) & 0x1;
    Byte msb = (shft_PatternMSB.value >> offset) & 0x1;

    return Utils::joinBits(msb,lsb);
}

void PPU::loadBackgroundPixel()
{
    Byte offset = 15 - XRegister;

    backgroundPaletteIndex = getBackgroundPalette(offset);
    backgroundColorId = getBackgroundColor(offset);

    Byte colorIndex = getNESPaletteColor(backgroundPaletteIndex,backgroundColorId,false);
    loadPixel(colorIndex);
}

void PPU::shiftBackgroundRegisters()
{
    if(maskRegister.renderBackground)
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
        if((scanline) >= OAM[spriteEvaluationIndex][0] && (scanline) <= (OAM[spriteEvaluationIndex][0] + 7 + 8* controlRegister.spriteSize))
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
                statusRegister.spriteOverflow = 1;
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
        {
            //Load [Pattern Low Byte] of the current sprite from Pattern Table using the pattern index from the secondary OAM
            int spriteRow = scanline - Secondary_OAM[spriteFetchingIndex][0];
            //8x16 sprites
            if(controlRegister.spriteSize)
            {
                //Sprite flipped vertically
                if(Secondary_OAM[spriteFetchingIndex][2] & 0x80)
                {
                    if(spriteRow > 7)
                    {
                        shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead(((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | ((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) << 4) | (7 - (spriteRow & 0x7)));
                    }
                    else
                    {
                        shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead(((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | (((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) + 1) << 4) | (7 - (spriteRow & 0x7)));
                    }
                }
                //Sprite NOT flipped
                else
                {
                    if(spriteRow > 7)
                    {
                        shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead(((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | (((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) + 1) << 4) | (spriteRow & 0x7));
                    }
                    else
                    {
                        shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead(((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | ((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) << 4) | (spriteRow & 0x7));
                    }
                }
            }
            //8x8 sprites
            else
            {
                //Sprite flipped vertically
                if(Secondary_OAM[spriteFetchingIndex][2] & 0x80)
                {
                    shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead((controlRegister.spriteAdress << 12) | (Secondary_OAM[spriteFetchingIndex][1] << 4) | (7 - spriteRow));
                }
                //Sprite NOT flipped
                else
                {
                    shft_SpritePatternLSB[spriteFetchingIndex].value = memoryRead((controlRegister.spriteAdress << 12) | (Secondary_OAM[spriteFetchingIndex][1] << 4) |  spriteRow);
                }
            }
            break;
        }
        case 7:
        {
            //Load [Pattern High Byte] of the current sprite from Pattern Table using the pattern index from the secondary OAM
            int spriteRow = scanline - Secondary_OAM[spriteFetchingIndex][0];
            //8x16 sprites
            if(controlRegister.spriteSize)
            {
                //Sprite flipped vertically
                if(Secondary_OAM[spriteFetchingIndex][2] & 0x80)
                {
                    if(spriteRow > 7)
                    {
                        //Bottom Half
                        shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead((((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | ((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) << 4) | (7 - (spriteRow & 0x7)))+ 8);
                    }
                    else
                    {
                        //Top Half
                        shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead((((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | (((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) + 1) << 4) | (7 - (spriteRow & 0x7)))+ 8);
                    }
                }
                //Sprite NOT flipped
                else
                {
                    if(spriteRow > 7)
                    {
                        //Bottom half
                        shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead((((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | (((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) + 1) << 4) | (spriteRow & 0x7))+ 8);
                    }
                    else
                    {
                        //Top half
                        shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead((((Secondary_OAM[spriteFetchingIndex][1] & 0x1) << 12) | ((Secondary_OAM[spriteFetchingIndex][1] & 0xFE) << 4) | (spriteRow & 0x7))+ 8);
                    }
                }
            }
            //8x8 sprites
            else
            {
                //Sprite flipped vertically
                if(Secondary_OAM[spriteFetchingIndex][2] & 0x80)
                {
                    shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead(((controlRegister.spriteAdress << 12) | (Secondary_OAM[spriteFetchingIndex][1] << 4) | (7 - spriteRow)) + 8);
                }
                //Sprite NOT flipped
                else
                {
                    shft_SpritePatternMSB[spriteFetchingIndex].value = memoryRead(((controlRegister.spriteAdress << 12) | (Secondary_OAM[spriteFetchingIndex][1] << 4) |  spriteRow) + 8);
                }
            }

            spriteFetchingIndex++;
            break;
        }
        }
    }
}

Byte PPU::getSpriteColorLSB(int i, bool spriteFlipped)
{
    Byte colorId = 0;
    if(spriteFlipped)   //Horizontally
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
    if(spriteFlipped)   //Horizontally
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
