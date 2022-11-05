#include "nes.h"
//#define PRINTCPU

NES::NES()
{
    systemCycles = 0;
    bus = new Bus;
    cartridge = new Cartridge;
    ppu = new PPU;
    cpu = new CPU(bus);
}

NES::~NES()
{
    bus->disconnectCartridge();
    ppu->disconnectCartridge();
    delete cartridge;

    bus->disconnectPPU();
    delete ppu;

    cpu->disconnectBUS();
    delete bus;

    delete cpu;
}

bool NES::loadGame(string path)
{
    if(cartridge->loadROM(path))
    {
        bus->connectCartridge(cartridge);


        ppu->connectCartridge(cartridge);
        bus->connectPPU(ppu);
        return true;
    }
    else
    {
        return false;
    }
}

void NES::run()
{
    //while(!cpu->HLT && systemCycles < 166539 * 3)
    //while(!cpu->HLT && systemCycles < 1814122 * 3)
    while(!cpu->HLT && systemCycles < 1611539 * 3)        //Para Super mario cargado el menu
    //while(!cpu->HLT && systemCycles < 10529989 * 3)         //Para Donkey Kong cargado el nivel
    {
        if(systemCycles >= 21)
        {
            ppu->executeCycle();
        }

        if(!((systemCycles+1) % 3))
        {
            cpu->executeCycle();
            if(cpu->readyToPrint)
            {
                cpu->readyToPrint = false;
                #ifdef PRINTCPU
                QString state = cpu->stringCPUState() + " " + ppu->stringPPUState() + " CYC: " + QString::number(cpu->getCycles());
                printf("%s\n",state.toUtf8().data());
                #endif
            }
        }

        if(ppu->NMI)
        {
            ppu->NMI = false;
            cpu->activateNMI();
        }

        systemCycles++;
    }

    ppu->drawNameTable();
    //ppu->drawPatternTable();
}

void NES::prueba()
{

}
