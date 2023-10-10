#ifndef NES_H
#define NES_H
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"

#ifdef RENDER_SCREEN
#include "SFML/Graphics.hpp"
#endif // RENDER_SCREEN

class NES
{
public:
    NES();
    ~NES();
    bool loadGame(std::string path);
    void run();
    void printState();

private:
    uint64_t systemCycles;
    uint64_t delayedFrames;
    Byte cycleCounter;

    //Pixel array
    Byte *pixels;

    //NES modules
    Bus *bus;
    Cartridge *cartridge;
    CPU *cpu;
    PPU *ppu;

#ifdef RENDER_SCREEN
    sf::RenderWindow window;
    sf::VideoMode desktop;
    sf::Texture pixels_texture;
    sf::Sprite pixels_sprite;
    sf::Event event;
    sf::Clock timer;
#endif // RENDER_SCREEN

};

#endif // NES_H
