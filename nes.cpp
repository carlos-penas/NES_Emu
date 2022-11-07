#include "nes.h"
#include "constants.h"

NES::NES()
{
    systemCycles = 0;
    bus = new Bus;
    cartridge = new Cartridge;
    ppu = new PPU;
    cpu = new CPU(bus);

#ifdef RENDERSCREEN
    //Set window actual resolution
    window.create(sf::VideoMode(PICTURE_WIDTH,PICTURE_HEIGHT),"Nintendo Entertainment System");

    //Increase window size if needed
    window.setSize(sf::Vector2u(PICTURE_WIDTH*RES_MULTIPLYER,PICTURE_HEIGHT*RES_MULTIPLYER));

    //Create pixel buffer with the appropriate size
    pixels = new Byte[PICTURE_WIDTH * PICTURE_HEIGHT * PIXEL_SIZE];

    //Create a texture with the same size as the window
    pixels_texture.create(PICTURE_WIDTH, PICTURE_HEIGHT);

    //Create a sf:Sprite to render the texture in the window
    pixels_sprite.setTexture(pixels_texture);

    //Load pixel buffer into ppu
    ppu->loadPixelBuffer(pixels);
#endif
}

NES::~NES()
{
    ppu->unloadPixelBuffer();
    delete[] pixels;

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
    //The reset routine is executed first (7 cpu cycles)
    while(cpu->getCycles() < 7)
    {
        cpu->executeCycle();
    }

    systemCycles = 21;

#ifdef PRINTLOG
    cpu->readyToPrint = false;
    QString state = cpu->stringCPUState() + " " + ppu->stringPPUState() + " CYC: " + QString::number(cpu->getCycles());
    printf("%s\n",state.toUtf8().data());
#endif

#ifdef RENDERSCREEN
    timer.start();
    while(!cpu->HLT && window.isOpen())
#endif
#ifndef RENDERSCREEN
    while(!cpu->HLT && systemCycles < 1611539 * 3)
    //while(!cpu->HLT && systemCycles < 10529989 * 3)
#endif
    {
        //Execute 1 ppu cycle every system cycle
        ppu->executeCycle();

        //Execute 1 cpu cycle every 3 system cycles
        if(!((systemCycles+1) % 3))
        {
            cpu->executeCycle();
#ifdef PRINTLOG
            if(cpu->readyToPrint)
            {
                cpu->readyToPrint = false;
                QString state = cpu->stringCPUState() + " " + ppu->stringPPUState() + " CYC: " + QString::number(cpu->getCycles());
                printf("%s\n",state.toUtf8().data());
            }
#endif
        }

        //Check if a NMI is needed
        if(ppu->NMI)
        {
            ppu->NMI = false;
            cpu->activateNMI();
        }

        //Increase system cycles
        systemCycles++;

#ifdef RENDERSCREEN
        //Render screen every ~16666666 ns (60 fps)
        if(timer.nsecsElapsed() >= 116666666)
        {
            //Prepare window for rendering
            pixels_texture.update(pixels);
            window.draw(pixels_sprite);

            window.display();
            timer.restart();
            //window.clear(sf::Color::Black);

            //Manage keyboard inputs
            while (window.pollEvent(event))
            {
                if(event.type == sf::Event::Closed)
                    window.close();
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)
                {
                    window.close();
                }
            }
        }
#endif

        //QUITAR CUANDO SE IMPLEMENTE EL RENDERIZADO POR CICLO EN LA PPU
        if(systemCycles == 4834617)
        //if(systemCycles == 31589967)
        {
            ppu->drawNameTable();
        }
    }
}

void NES::prueba()
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
                #ifdef PRINTLOG
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
