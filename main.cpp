#include "nes.h"

int main(int argc,char* argv[])
{
    string path = "";

    const string programName = "NES_Emu";
    const string programVersion = "1.0";
    const string author = "Carlos Penas"; //(Carlos Peñas)
    const string description = "Un emulador de la NES en C++ y la libreria SFML";
    const string githubURL = "https://github.com/carlos-penas/NES_Emu";

    std::cout << "Bienvenido a " << programName << " v" << programVersion << std::endl;
    std::cout << "Autor: " << author << std::endl;
    std::cout << "Descripción: " << description << std::endl;
    std::cout << "Repositorio en GitHub: " << githubURL << std::endl << std::endl;


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
