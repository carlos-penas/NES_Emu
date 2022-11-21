#ifndef NES_H
#define NES_H
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"
#include "compilationSettings.h"
#include <QElapsedTimer>

#ifdef RENDERSCREEN
#include "Graphics.hpp"
#endif

class NES
{
public:
    NES();
    ~NES();
    bool loadGame(string path);
    void run();
    void prueba();

private:
    uint64_t systemCycles;
    Byte cycleCounter;

    //Screen rendering
#ifdef RENDERSCREEN
    sf::RenderWindow window;
    sf::Texture pixels_texture;
    sf::Sprite pixels_sprite;
    sf::Event event;
    QElapsedTimer timer;
#endif
    Byte *pixels;

    //NES modules
    Bus *bus;
    Cartridge *cartridge;
    CPU *cpu;
    PPU *ppu;
};

#endif // NES_H
