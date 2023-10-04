#include "nes.h"
#include "ProjectConfig.h"

int main(int argc,char* argv[])
{
    string path = "";
    const string author = "Carlos Penas"; //(Carlos Peñas)

    std::cout << "Bienvenido a " << PROJECT_NAME << " v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << std::endl;
    std::cout << "Autor: " << author << std::endl;
    std::cout << "Descripcion: " << PROJECT_DESCRIPTION << std::endl;
    std::cout << "Repositorio en GitHub: " << GITHUB_URL << std::endl << std::endl;


    if(argc == 1)
    {
        printf("ERROR: Se debe especificar una ROM\n");
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
