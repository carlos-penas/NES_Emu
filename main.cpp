#include "nes.h"

int main(int argc,char* argv[])
{
    string path = "";

    setlocale(LC_ALL, "spanish");
    printf("EMULADOR DE LA NES\n");
    printf("\tpor Carlos Peñas (carlos.penas.n@gmail.com).\n\n");
    printf("Repositorio del proyecto: https://github.com/carlos-penas/NES_Emu\n\n");

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
