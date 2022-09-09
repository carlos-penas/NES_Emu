#include <cpu.h>
#include "ppu.h"
#include <cartridge.h>

#define ROMPATH2 "/home/carlos/programming/NES_Emulator/Documents/nestest.nes"

int main(int argc,char* argv[])
{
    string path = "";

    if(argc == 1)
    {
        path = ROMPATH2;
    }else
    {
        path = argv[1];
    }

    Bus bus;
    Cartridge cartridge;
    if(cartridge.loadROM(path))
    {
        bus.connectCartridge(&cartridge);
        CPU cpu(&bus);
        PPU ppu(&bus);
        ppu.connectCartridge(&cartridge);
        ppu.drawPatternTable();
        //cpu.run();
    }
    else
    {
        printf("Error al cargar la ROM\n");
    }

    return 0;
}
