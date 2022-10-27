#ifndef NES_H
#define NES_H
#include "cartridge.h"
#include "cpu.h"
#include "ppu.h"


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

    Bus *bus;
    Cartridge *cartridge;
    CPU *cpu;
    PPU *ppu;
};

#endif // NES_H
