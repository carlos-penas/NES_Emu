#include <cpu.h>
#include <iostream>
#include <fstream>

#define ROMPATH2 "/home/carlos/programming/NES_Emulator/Documents/nestest.nes"

using std::cout;
using std::endl;
using std::string;
using std::ifstream;


int main(int argc,char* argv[])
{

    CPU cpu;

    if(argc == 1)
    {
        ifstream file(ROMPATH2);
        //ROM is read to the buffer
        unsigned char buffer2[24592];
        file.read((char*)buffer2,sizeof(buffer2));
        cpu.loadProgram(buffer2,0x4010);

    }else
    {
        ifstream file(argv[1]);
        unsigned char header[16];
        file.read((char*)header,sizeof(header));

//        for(int i=0; i< sizeof(header);i++)
//        {
//            printf("%02X - %02X \n", i,header[i]);
//        }
        uint64_t PRG_ROM_Size = 16384 * header[4];
        cout << "Es" << PRG_ROM_Size << endl;
        //printf("\nEl Byte 4 es %02X --> Tamaño: %ld bytes", header[4], (long)PRG_ROM_Size);



        unsigned char PRG_ROM[0xBFE0];

        file.read((char*)PRG_ROM,PRG_ROM_Size);
//        for(int i=0; i< sizeof(header);i++)
//        {
//            printf("%02X - %02X \n", i,buffer2[i]);
//        }
        cpu.loadProgram(PRG_ROM,PRG_ROM_Size);
    }



    cpu.run();

    return 0;
}
