#include "nes.h"
#include "constants.h"

NES::NES()
{
    systemCycles = 0;
    cycleCounter = 3;
    delayedFrames = 0;
    event = sf::Event();
    bus = new Bus;
    cartridge = new Cartridge;
    ppu = new PPU;
    cpu = new CPU(bus);

#ifdef RENDERSCREEN
    //Load icon from png
    //sf::Image icon;
    //icon.loadFromFile("/home/carlos/programming/NES_Emulator/Documents/NES_Icon.png");

    //Set window actual resolution
    window.create(sf::VideoMode(PICTURE_WIDTH,PICTURE_HEIGHT),"Nintendo Entertainment System");

    //Increase window size if needed
    window.setSize(sf::Vector2u(PICTURE_WIDTH*RES_MULTIPLYER,PICTURE_HEIGHT*RES_MULTIPLYER));

    //Set window icon
    //window.setIcon(icon.getSize().x,icon.getSize().y,icon.getPixelsPtr());

    //Center Screen
    desktop = sf::VideoMode::getDesktopMode();
    window.setPosition(sf::Vector2i(desktop.width/2 - window.getSize().x/2,desktop.height/2 - window.getSize().y/2));
#endif
    //Create pixel buffer with the appropriate size
    pixels = new Byte[PICTURE_WIDTH * PICTURE_HEIGHT * PIXEL_SIZE];

#ifdef RENDERSCREEN
    //Create a texture with the same size as the window
    pixels_texture.create(PICTURE_WIDTH, PICTURE_HEIGHT);

    //Create a sf:Sprite to render the texture in the window
    pixels_sprite.setTexture(pixels_texture);

    //Start with a black screen
    window.clear(sf::Color::Black);
    window.display();

#endif
    //Load pixel buffer into ppu
    ppu->loadPixelBuffer(pixels);
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
    string state = cpu->stringCPUState() + " " + ppu->stringPPUState() + " CYC: ";
    printf("%s%ld\n",state.data(),cpu->getCycles());
#endif

#ifdef RENDERSCREEN
    timer.restart();
    while(!cpu->HLT && window.isOpen())
#endif
#ifndef RENDERSCREEN
    //while(!cpu->HLT && systemCycles < 1611539 * 3)
    while(!cpu->HLT && systemCycles < 1250 * 3)
    //while(!cpu->HLT && systemCycles < 3968225)
    //while(!cpu->HLT && systemCycles < 17062759 * 3)
#endif
    {
        //Execute 1 ppu cycle every system cycle
        ppu->executeCycle();

        cycleCounter--;

        //Execute 1 cpu cycle every 3 system cycles
        if(cycleCounter == 0)
        {
            cycleCounter = 3;
            cpu->executeCycle();
#ifdef PRINTLOG
            if(cpu->readyToPrint)
            {
                cpu->readyToPrint = false;
                string state = cpu->stringCPUState() + " " + ppu->stringPPUState() + " CYC: ";
                printf("%s%ld\n",state.data(),cpu->getCycles());
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
        //When the ppu reports that the frame is complete, the screen is refreshed
        if(ppu->frameComplete)
        {
            ppu->frameComplete = false;

            //Prepare window for rendering
            pixels_texture.update(pixels);
            window.draw(pixels_sprite);

            if(timer.getElapsedTime().asMicroseconds() > 16666)
            {
                delayedFrames++;
                printf("ATENCION, SE ESTÁ RALENTIZANDO EL JUEGO\n");
            }
            //printf("T = %lld\n", timer.nsecsElapsed());

            //Render screen every ~16666 us (for 60 fps)
            while(timer.getElapsedTime().asMicroseconds() < 16666)
            {
                continue;
            }

            window.display();
            timer.restart();
            window.clear(sf::Color::Black);

            //Manage keyboard inputs
            while (window.pollEvent(event))
            {
                //Close the window when pressing the X on the window
                if(event.type == sf::Event::Closed)
                    window.close();
                //Close the window when pressing the ESC key
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
                //Resize the window to x2 the original NES size when pressing the F1 key
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F1)
                {
                    window.setSize(sf::Vector2u(PICTURE_WIDTH*2,PICTURE_HEIGHT*2));
                    window.setPosition(sf::Vector2i(desktop.width/2 - window.getSize().x/2,desktop.height/2 - window.getSize().y/2));
                }
                //Resize the window to x3 the original NES size when pressing the F2 key
                else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F2)
                {
                    window.setSize(sf::Vector2u(PICTURE_WIDTH*3,PICTURE_HEIGHT*3));
                    window.setPosition(sf::Vector2i(desktop.width/2 - window.getSize().x/2,desktop.height/2 - window.getSize().y/2));
                }
            }
        }
#endif
    }

    printf("Cycles executed: %ld\n",systemCycles);
    printf("Delayed frames: %ld\n", delayedFrames);
}
