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
    while(!cpu->HLT && systemCycles < 1611539 * 3)
    //while(!cpu->HLT && systemCycles < 50000)
    {
        if(!(systemCycles % 3))
        {
            cpu->executeCycle();
            if(cpu->readyToPrint)
            {
                cpu->readyToPrint = false;
                #ifdef PRINTCPU
                QString state = cpu->stringCPUState() + " " + ppu->stringPPUState() + " CYC: " + QString::number(cpu->getCycles()-1);
                printf("%s\n",state.toUtf8().data());
                #endif
            }
        }

        if(systemCycles >= 21)
        {
            ppu->executeCycle();
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
