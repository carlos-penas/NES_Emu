#include "nes.h"

int main(int argc,char* argv[])
{
    string path = "";

    if(argc == 1)
    {
        printf("Se debe especificar una ROM\n");
        return 0;
    }else
    {
        path = argv[1];
    }

    NES nes;

    if(nes.loadGame(path))
    {
        nes.run();
        printf("\nHalting the system...\n\n");
    }
    else
    {
        printf("Error al cargar la ROM\n");
    }

    return 0;
}
