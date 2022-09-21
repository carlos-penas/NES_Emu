#include "ppu.h"
#include "constants.h"
#include <cstring>
#include <unistd.h>

PPU::PPU(Bus *bus)
{
    this->bus = bus;

    //Set window actual resolution
    window.create(sf::VideoMode(PT_TABLE_WIDTH,PT_TABLE_HEIGHT),"NES Emulator");

    //Increase window size if needed
    window.setSize(sf::Vector2u(PT_TABLE_WIDTH*RES_MULTIPLYER,PT_TABLE_HEIGHT*RES_MULTIPLYER));
}

void PPU::connectCartridge(Cartridge *cartridge)
{
    this->cartridge = cartridge;
}

void PPU::drawFrame()
{
    while(window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);


        //Draw here.


        window.display();
    }
}

void PPU::drawPatternTable()
{
    //Create pixel buffer with the appropriate size
    Byte *pixels = new Byte[PT_TABLE_WIDTH * PT_TABLE_HEIGHT * PIXEL_SIZE];

    //Create a texture with the same size as the window
    sf::Texture pixels_texture;
    pixels_texture.create(PT_TABLE_WIDTH, PT_TABLE_HEIGHT);

    //Create a sf:Sprite to render the texture in the window
    sf::Sprite pixels_sprite(pixels_texture);
    bool ya = false;

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

        uint64_t pixelToLoad = 0;

        if(!ya)
        {
            for(int tableRow = 0; tableRow < PT_TABLE_ROWS; tableRow++)
            {
                for(int tableCol = 0; tableCol < PT_TABLE_COLS; tableCol ++)
                {
                    //Load each pattern from the Pattern table into the pixel buffer
                    loadPattern(tableRow,tableCol,&pixels[pixelToLoad]);

                    //Update pixel pointer to the position of the next pattern
                    pixelToLoad += PTRN_LNGHT * PIXEL_SIZE;
                }
                //Update pixel pointer to the position of the next pattern (on the next row)
                pixelToLoad += PTRN_LNGHT * PT_TABLE_COLS * (PTRN_LNGHT-1) * PIXEL_SIZE;
            }
            ya = true;
        }

        //Update the texture of the sf:Sprite with the pixel buffer
        pixels_texture.update(pixels);
        window.draw(pixels_sprite);


        window.display();
    }
}

void PPU::loadPattern(int tableRow, int tableCol, Byte *pixels)
{
    //Get the Pattern Table index of the pattern that is going to get loaded
    int patternIndex = getPatternIndex(tableRow,tableCol);

    //Load each row of pixels of the pattern individually
    for(int i = 0; i < PTRN_LNGHT; i++)
    {
        loadPatternRow(pixels,i,patternIndex);
    }
}

void PPU::loadPatternRow(Byte *pixels, int ptrn_row, int patternIndex)
{
    //Get from the Pattern Table both bit planes of the pattern row that is going to get loaded
    Byte plane0Row = cartridge->PPU_Read(patternIndex*PTRN_SIZE + ptrn_row);
    Byte plane1Row = cartridge->PPU_Read(patternIndex*PTRN_SIZE + (PTRN_SIZE/2) + ptrn_row);

    //Load each pixel of the row individually into the pixel buffer with the specified color index (from right to left)
    for(int i = 0; i < PTRN_LNGHT; i++)
    {
        Byte pixel0 = (plane0Row >> i) & 0x01;
        Byte pixel1 = (plane1Row >> i) & 0x01;

        Byte colorIndex = (pixel1 << 1) | pixel0;

        if(colorIndex == 0x0)
            memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],color1,sizeof(color1));
        if(colorIndex == 0x1)
            memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],color2,sizeof(color2));
        if(colorIndex == 0x2)
            memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],color3,sizeof(color3));
        if(colorIndex == 0x3)
            memcpy(&pixels[ptrn_row * PT_TABLE_COLS * PTRN_LNGHT * PIXEL_SIZE + (PTRN_LNGHT-1-i) * PIXEL_SIZE],color4,sizeof(color4));
    }

}

int PPU::getPatternIndex(int tableRow, int tableCol)
{
    return PT_TABLE_COLS * tableRow + tableCol;
}


//Siguiente:
//Implementar interrupciones en la cpu. Ya se como hacer para que interaccione (clase NES??).
//La ppu necesita un metodo: Acceso a memoria. Y ya dependiendo de eso va al cartucho o a lo que sea. Hay que comporobar las direcciones de memoria
//que aparecen aquí: https://www.nesdev.org/wiki/PPU_pattern_tables (ultima linea) . De CHR_ROM. Están bien al acceder a ellas?? etc.
