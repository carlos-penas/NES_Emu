#include "ppu.h"
#include "constants.h"
#include <cstring>
#include <unistd.h>
#include "compilationSettings.h"

PPU::PPU()
{
    PPUSTATUS.VBlank = 0;
    //PPUSTATUS =0xA0;
    PPUSTATUS.value =0x00;
    PPUCTRL.value = 0;
    OAMADDR = 0;

    internalBuffer = 0;

    cycle = 0;
    scanline = 0;

    patternIndex = 0;
    currentPixel = 0;

    pixelOffsetX = 0;

    patternLSB = 0;

    NMI = false;
    frameComplete = false;
    oddFrame = false;
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
        //If we are on a part of the screen that is not blank (1-256), we render the pixel
        if(cycle <= 256 && cycle >=1 && renderEnabled())
        {
            Byte offset = 15 - pixelOffsetX;

            Byte LSB = (shft_PaletteLow >> offset) & 0x1;
            Byte MSB = (shft_PaletteHigh >> offset) & 0x1;

            paletteRAMIndex = (MSB << 1) | LSB;

            LSB = (shft_PatternLSB >> offset) & 0x1;
            MSB = (shft_PatternMSB >> offset) & 0x1;

            colorOffset = (MSB << 1) | LSB;

            Byte colorIndex = getColor(paletteRAMIndex,colorOffset);

            memcpy(&pixels[(scanline * PICTURE_WIDTH + cycle-1) * PIXEL_SIZE],&NESPallette[colorIndex][0],PIXEL_SIZE);
        }

        if((cycle <= 256 && cycle >= 1) || (cycle >= 328 && cycle <=336))
        {
            //Each cycle, the shift registers are shifted
            shiftRegisters();
        }

        if((cycle <= 256 && cycle >= 2) || (cycle >= 322 && cycle <=336))
        {
            //This sequence is repeated every 8 cycles to load all the data for the current tile
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
                    loadShiftRegisters();

                    //Increment x component of VRAM
                    incrementCurrentX();
                }
                break;
            }
        }

        //At cycle 256 of every scanline, the Y component of the current VRAM address is incremented
        if(cycle == 256 && renderEnabled())
        {
            incrementCurrentY();
        }

        //At cycle 257 of every scanline, the X component of the current VRAM address is updated with the contents of the temporal VRAM address
        if(cycle == 257 && renderEnabled())
        {
            updateCurrentX();
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

        //if((cycle >= 1 && cycle <= 256) || (cycle >= 328 && cycle <=336))
        if((cycle >= 328 && cycle <=336))
        {
            //Each cycle, the shift registers are shifted
            shiftRegisters();
        }

        if((cycle <= 256 && cycle >= 2) || (cycle >= 322 && cycle <=336))
        {
            //This sequence is repeated every 8 cycles to load all the data for the current tile
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
                    loadShiftRegisters();

                    //Increment x component of VRAM
                    incrementCurrentX();
                }
                break;
            }
        }

        //At cycle 256 of every scanline, the Y component of the current VRAM address is incremented
        if(cycle == 256 && renderEnabled())
        {
            incrementCurrentY();
        }

        //At cycle 257 of every scanline, the X component of the current VRAM address is updated with the contents of the temporal VRAM address
        if(cycle == 257 && renderEnabled())
        {
            updateCurrentX();
        }

        //During cycles 280-304 of the pre-render scanline, the X component of the VRAM address is repeatedly updated with the contents of the temporal VRAM address
        if(cycle >= 280 && cycle <= 304 && renderEnabled())
        {
            updateCurrentY();
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

    //The first cycle of the next frame is skipped if its an odd frame
    oddFrame = !oddFrame;
    if(oddFrame && cycle == 340 && scanline == 261 && renderEnabled())
    {
        cycle = 0;
        scanline = 0;
    }

}

void PPU::drawFrame()
{
//    while(window.isOpen())
//    {
//        sf::Event event;
//        while (window.pollEvent(event))
//        {
//            if(event.type == sf::Event::Closed)
//                window.close();
//        }

//        window.clear(sf::Color::Black);


//        //Draw here.


//        window.display();
//    }
}

void PPU::drawPatternTable()
{
    //Create pixel buffer with the appropriate size
//    Byte *pixels = new Byte[PT_TABLE_WIDTH * PT_TABLE_HEIGHT * PIXEL_SIZE];

//    //Create a texture with the same size as the window
//    sf::Texture pixels_texture;
//    pixels_texture.create(PT_TABLE_WIDTH, PT_TABLE_HEIGHT);

//    //Create a sf:Sprite to render the texture in the window
//    sf::Sprite pixels_sprite(pixels_texture);
//    bool ya = false;

//    while(window.isOpen())
//    {
//        usleep(16666);
//        sf::Event event;
//        while (window.pollEvent(event))
//        {
//            if(event.type == sf::Event::Closed)
//                window.close();
//            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
//                window.close();
//            }
//        }

//        window.clear(sf::Color::Black);

//        uint64_t pixelToLoad = 0;

//        if(!ya)
//        {
//            for(int tableRow = 0; tableRow < PT_TABLE_ROWS; tableRow++)
//            {
//                for(int tableCol = 0; tableCol < PT_TABLE_COLS; tableCol ++)
//                {
//                    //Load each pattern from the Pattern table into the pixel buffer
//                    loadPattern(tableRow,tableCol,&pixels[pixelToLoad]);

//                    //Update pixel pointer to the position of the next pattern
//                    pixelToLoad += PTRN_LNGHT * PIXEL_SIZE;
//                }
//                //Update pixel pointer to the position of the next pattern (on the next row)
//                pixelToLoad += PTRN_LNGHT * PT_TABLE_COLS * (PTRN_LNGHT-1) * PIXEL_SIZE;
//            }
//            ya = true;
//        }

//        //Update the texture of the sf:Sprite with the pixel buffer
//        pixels_texture.update(pixels);
//        window.draw(pixels_sprite);


//        window.display();
//        }
}

void PPU::drawNameTable()
{
    int nametableIndex = 0x2000;
    int attributeTableIndex = 0;
    int blockIndex = 0;
    uint64_t pixelToLoad = 0;

    for(int gridRow = 0; gridRow < BACKGROUND_ROWS; gridRow++)
    {
        for(int gridCol = 0; gridCol < BACKGROUND_COLS; gridCol ++)
        {
            blockIndex = (gridRow & 0x2) | (gridCol & 0x2) >> 1;

            //Read entry from the Nametable
            int patternIndex = memoryRead(nametableIndex) + 0x100;  //+100 porque son los del backround (estan en la segunda mitad de la tabla (Cada tabla son 0x100 tiles).

            //Read entry from Attribute table
            int blockAttribute = memoryRead(0x2000 + 0x3C0 + attributeTableIndex);

            int paletteIndex = getPaletteIndex(blockIndex,blockAttribute);

            //Load each row of pixels of the pattern individually
            for(int i = 0; i < PTRN_LNGHT; i++)
            {
                loadPatternRow(&pixels[pixelToLoad],i,patternIndex,paletteIndex);
            }

            //Update pixel pointer to the position of the next pattern
            pixelToLoad += PTRN_LNGHT * PIXEL_SIZE;

            //Increment indexes if needed
            nametableIndex++;

            if(!((gridCol+1) % 4))
                attributeTableIndex++;


        }
        //Update pixel pointer to the position of the next pattern (on the next row)
        pixelToLoad += PTRN_LNGHT * BACKGROUND_COLS * (PTRN_LNGHT-1) * PIXEL_SIZE;

        //Reset attribute table index if needed
        if(((gridRow+1) % 4))
            attributeTableIndex = attributeTableIndex - 8;

    }
    //    printf("OAM:\n");
    //    for(int i = 0; i < 256; i++)
    //    {
    //        printf("OAM[%02X] = %02X\n",i,OAM[i]);
    //        if(!((i+1) % 4))
    //        {
    //            printf("\n");
    //        }
    //    }
}
void PPU::loadPattern(int tableRow, int tableCol, Byte *pixels)
{
    //Get the Pattern Table index of the pattern that is going to get loaded
    int patternIndex = getPatternIndex(tableRow,tableCol);

    //Load each row of pixels of the pattern individually
    for(int i = 0; i < PTRN_LNGHT; i++)
    {
        loadPatternRow(pixels,i,patternIndex,0);
    }
}

void PPU::loadPatternRow(Byte *pixels, int ptrn_row, int patternIndex, int paletteIndex)
{
    //Get from the Pattern Table both bit planes of the pattern row that is going to get loaded
    Byte plane0Row = memoryRead(patternIndex*PTRN_SIZE + ptrn_row);
    Byte plane1Row = memoryRead(patternIndex*PTRN_SIZE + (PTRN_SIZE/2) + ptrn_row);

    //Load each pixel of the row individually into the pixel buffer with the specified color index (from right to left)
    for(int i = 0; i < PTRN_LNGHT; i++)
    {
        Byte pixel0 = (plane0Row >> i) & 0x01;
        Byte pixel1 = (plane1Row >> i) & 0x01;

        Byte colorOffset = (pixel1 << 1) | pixel0;
        Byte colorIndex = getColor(paletteIndex,colorOffset);

//        if(colorIndex == 0x0)
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x22][0],4);
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[15][0],4);
//        if(colorIndex == 0x1)
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x29][0],4);
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x11][0],4);
//        if(colorIndex == 0x2)
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x1A][0],4);
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x37][0],4);
//        if(colorIndex == 0x3)
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x0F][0],4);
//            //memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[0x16][0],4);
        memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],&NESPallette[colorIndex][0],PIXEL_SIZE);
    }

}

int PPU::getPatternIndex(int tableRow, int tableCol)
{
    return PT_TABLE_COLS * tableRow + tableCol;
}

Byte PPU::getColor(int paletteIndex, Byte colorOffset)
{
    int index = paletteIndex*4 + colorOffset;

    if(index == 0x04 || index == 0x08 || index == 0x0C)
        return memoryRead(0x3F00);
    else
        return memoryRead(0x3F00 + index);
}

//Memory
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

void PPU::incrementCurrentY()
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

void PPU::incrementCurrentX()
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

void PPU::updateCurrentY()
{
    VRAMAdress.tileOffsetY = tempVRAMAdress.tileOffsetY;
    VRAMAdress.nametableY = tempVRAMAdress.nametableY;
    VRAMAdress.pixelOffsetY = tempVRAMAdress.pixelOffsetY;
}

void PPU::updateCurrentX()
{
    VRAMAdress.tileOffsetX = tempVRAMAdress.tileOffsetX;
    VRAMAdress.nametableX = tempVRAMAdress.nametableX;
}

void PPU::shiftRegisters()
{
    shft_PatternLSB <<= 1;
    shft_PatternMSB <<= 1;
    shft_PaletteLow <<= 1;
    shft_PaletteHigh <<= 1;
}

void PPU::loadShiftRegisters()
{
    //Load data for the next pattern on the LSB of the shift registers
    shft_PatternLSB &= 0xFF00;
    shft_PatternMSB &= 0xFF00;
    shft_PatternLSB |= patternLSB;
    shft_PatternMSB |= patternMSB;

    //Load the (same) palette index on the LSB of the shift registers
    if(tileAttribute & 0b01)
        shft_PaletteLow |= 0xFF;
    else
        shft_PaletteLow |= 0x00;

    if(tileAttribute & 0b10)
        shft_PaletteHigh |= 0xFF;
    else
        shft_PaletteHigh |= 0x00;
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
        break;
    }
    case RegAddress::OAMDATA:
    {
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

void PPU::OAM_DMA_Transfer(Byte value, int index)
{
    OAM[(OAMADDR + index) & 0xFF] = value;
}

QString PPU::stringPPUState()
{
    return QString("PPU %1,%2").arg(cycle,3,10,QLatin1Char(' ')).arg(scanline,3,10,QLatin1Char(' '));
}

Byte PPU::getPaletteIndex(int blockIndex, int blockAttribute)
{
    Byte paleteIndex = blockAttribute >> blockIndex * 2;
    return paleteIndex & 0x03;
}
