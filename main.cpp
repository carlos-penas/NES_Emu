#include "nes.h"
#include "ProjectConfig.h"

int main(int argc,char* argv[])
{
    std::string path = "";
    const std::string author = "Carlos Penas"; //(Carlos Peñas)

    std::cout << "Bienvenido a " << PROJECT_NAME << " v" << PROJECT_VERSION_MAJOR << "." << PROJECT_VERSION_MINOR << std::endl;
    std::cout << PROJECT_DESCRIPTION << std::endl;
    std::cout << "Autor: " << author << std::endl;
    std::cout << "Repositorio en GitHub: " << GITHUB_URL << std::endl << std::endl;


    if(argc == 1)
    {
        std::cout << "ERROR: Se debe especificar una ROM" << std::endl;
        return 0;
    }else
    {
        path = argv[1];
    }

    NES nes;

    if(nes.loadGame(path))
    {
        nes.run();
        std::cout << "Deteniendo el sistema..." << std::endl << std::endl;
    }
    else
    {
        std::cout << "Error al cargar la ROM" << std::endl << std::endl;
    }

    return 0;
}
