#include <cpu.h>
#include <iostream>
//#include <string>
#include <fstream>

#define ROMPATH2 "/home/carlos/programming/NES_Emulator/Documents/nestest.nes"

using std::cout;
using std::endl;
using std::string;
using std::ifstream;

int main()  //main(int argc, char *argv[])
{

    CPU cpu;

    ifstream file(ROMPATH2);

    //ROM is read to the buffer
    unsigned char buffer2[24592];
    file.read((char*)buffer2,sizeof(buffer2));

    cpu.loadProgram(buffer2,0x4010);

    cpu.run();

    return 0;
}
