#include "ppu.h"

PPU::PPU(Bus *bus)
{
    this->bus = bus;

    window.create(sf::VideoMode(256,240),"NES Emulator");

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
