#ifndef NES_H
#define NES_H
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "compilationSettings.h"

#ifdef RENDERSCREEN
#include "SFML/Graphics.hpp"
#endif

class NES
{
public:
    NES();
    ~NES();
    bool loadGame(string path);
    void run();

private:
    uint64_t systemCycles;
    uint64_t delayedFrames;
    Byte cycleCounter;

    //Screen rendering
#ifdef RENDERSCREEN
    sf::RenderWindow window;
    sf::VideoMode desktop;
    sf::Texture pixels_texture;
    sf::Sprite pixels_sprite;
    sf::Event event;
    sf::Clock timer;
#endif
    Byte *pixels;

    //NES modules
    Bus *bus;
    Cartridge *cartridge;
    CPU *cpu;
    PPU *ppu;
};

#endif // NES_H
