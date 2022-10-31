#include "ppu.h"
#include "constants.h"
#include <cstring>
#include <unistd.h>
//#define PRINTPPU


PPU::PPU()
{
    //Set window actual resolution
    //window.create(sf::VideoMode(PT_TABLE_WIDTH,PT_TABLE_HEIGHT),"Pattern Table");      //Pattern Table
    window.create(sf::VideoMode(PICTURE_WIDTH,PICTURE_HEIGHT),"Nintendo Entertainment System");      //NES screen

    //Increase window size if needed
    //window.setSize(sf::Vector2u(PT_TABLE_WIDTH*RES_MULTIPLYER,PT_TABLE_HEIGHT*RES_MULTIPLYER));
    window.setSize(sf::Vector2u(PICTURE_WIDTH*RES_MULTIPLYER,PICTURE_HEIGHT*RES_MULTIPLYER));

    PPUADDR.latch = 0;
    PPUADDR.address = 0;
    //PPUSTATUS =0xA0;
    PPUSTATUS =0x00;
    PPUCTRL = 0;
    OAMADDR = 0;

    internalBuffer = 0;

    cycle = 0;
    scanline = 0;

    NMI = false;
}

void PPU::connectCartridge(Cartridge *cartridge)
{
    this->cartridge = cartridge;
}

void PPU::disconnectCartridge()
{
    this->cartridge = NULL;
}

void PPU::executeCycle()
{
    //At the start of the frame, the Vertical Blank flag is reset
    if(scanline == 1 && cycle == 1)
        PPUSTATUS_SetVBlank(false);


    //From scanline 241 onwards, the Vertical Blank flag is set
    if(scanline == 241 && cycle == 1)
    {
        PPUSTATUS_SetVBlank(true);
        if(PPUCTRL_NMI())
        {
            NMI = true;
        }
    }

    cycle++;
    if(cycle == 342)
    {
        cycle = 1;
        scanline++;
    }

    if(scanline == 262)
    {
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

void PPU::drawNameTable() //Byte *pixels parametro.
{
    //Create pixel buffer with the appropriate size
    Byte *pixels = new Byte[PICTURE_WIDTH * PICTURE_HEIGHT * PIXEL_SIZE];

    //Create a texture with the same size as the window
    sf::Texture pixels_texture;
    pixels_texture.create(PICTURE_WIDTH, PICTURE_HEIGHT);

    //Create a sf:Sprite to render the texture in the window
    sf::Sprite pixels_sprite(pixels_texture);
    bool ya = false;

    //initializePallette();

    while(window.isOpen())
    {
        usleep(16666);
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                window.close();
            }
        }

        window.clear(sf::Color::Black);

        int nametableIndex = 0x2000;
        int attributeTableIndex = 0;
        int blockIndex = 0;
        uint64_t pixelToLoad = 0;

        if(!ya)
        {
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

            ya = true;
        }

        //Update the texture of the sf:Sprite with the pixel buffer
        pixels_texture.update(pixels);
        window.draw(pixels_sprite);


        window.display();
    }

    delete[] pixels;

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
        loadPatternRow(pixels,i,patternIndex,1);
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
    if(address >= 0x2000 && address <= 0x3EFF)
    {
#ifdef PRINTPPU
        printf("Escritura [%04X] --> Nametable[%d][%04X]\n", realAddress, (address >> 10) & 0x03, (address & 0x3FF));
#endif
        NameTables[(address >> 10) & 0x03][address & 0x3FF] = value;
    }

    //Pallette Indexes and their mirrors
    if(address >= 0x3F00 && address <= 0x3FFF)
    {
#ifdef PRINTPPU
        printf("Escritura [%04X] --> Palette Index [%04X]\n", realAddress, address & 0x1F);
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
    if(address >= 0x2000 && address <= 0x3EFF)
    {
        if(cartridge->getMirroringType() == HorizontalMirroring)
        {
#ifdef PRINTPPU
            printf("Lectura [%04X] -->  Nametable[%d][%04X]\n", realAddress, (address >> 11) & 1, address & 0x03FF);
#endif
            return NameTables[(address >> 11) & 1][address & 0x03FF];
        }
        else if(cartridge->getMirroringType() == VerticalMirroring)
        {
#ifdef PRINTPPU
            printf("Lectura [%04X] -->  Nametable[%d][%04X]\n", realAddress, (address >> 10) & 1, address & 0x03FF);
#endif
            return NameTables[(address >> 10) & 1][address & 0x03FF];
        }
    }

    //Pallette Indexes and their mirrors
    if(address >= 0x3F00 && address <= 0x3FFF)
    {
#ifdef PRINTPPU
        printf("Lectura [%04X] --> Palette Index [%04X]\n", realAddress, address & 0x1F);
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

bool PPU::PPUCTRL_VRAM_increment()
{
    return (PPUCTRL & 0b00000100);
}

bool PPU::PPUCTRL_NMI()
{
    return (PPUCTRL & 0b10000000);
}

void PPU::PPUSTATUS_SetVBlank(bool set)
{
    if(set)
        PPUSTATUS |= 0b10000000;
    else
        PPUSTATUS &= 0b01111111;
}


void PPU::cpuWrite(Byte value, Address address)
{
    switch (address) {
    case RegAddress::PPUCTRL:
    {
        PPUCTRL = value;
        break;
    }
    case RegAddress::PPUMASK:
    {
        PPUMASK = value;
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
        break;
    }
    case RegAddress::PPUADDR:
    {
        if(PPUADDR.latch_isSet())
        {
            PPUADDR.setLowByte(value);
            PPUADDR.resetLatch();
        }
        else //if latch is reset
        {
            PPUADDR.setHighByte(value);
            PPUADDR.setLatch();
        }
        break;
    }
    case RegAddress::PPUDATA:
    {
        //Write the value to the address stored in PPUADDR
        memoryWrite(value,PPUADDR.address);

        //The address stored in PPUADDR (VRAM address) is incremented by the amount specified in bit 2 of PPUCTRL ('0' --> increment by 1, '1' --> increment by 32)
        if(PPUCTRL_VRAM_increment())
            PPUADDR.address += 32;
        else
            PPUADDR.address++;
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
        Byte result = (PPUSTATUS & 0xE0) | (internalBuffer & 0x1F);
        PPUADDR.resetLatch();
        PPUSTATUS_SetVBlank(false);
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
        Byte readData = memoryRead(PPUADDR.address);
        Byte result;

        //If the address is within the palette range, we return the read value directly. Otherwise, we return the value of the previous read stored on the internal buffer.
        if((PPUADDR.address & 0x3FFF) >= 0x3F00)
            result = readData;
        else
            result = internalBuffer;

        //The internal buffer is always updated with the read data
        internalBuffer = readData;

        //The address stored in PPUADDR (VRAM address) is incremented by the amount specified in bit 2 of PPUCTRL ('0' --> increment by 1, '1' --> increment by 32)
        if(PPUCTRL_VRAM_increment())
            PPUADDR.address += 32;
        else
            PPUADDR.address++;

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
