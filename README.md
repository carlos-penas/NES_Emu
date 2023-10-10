# NES Emulator
An emulator for the videogame console NES (*Nintendo Entertainment System*) developed in C++ and the graphics library SFML, originally developed as the final project of my software engineering degree.

The emulator currently supports Mapper 0 and NTSC video system, which means it is compatible with ~100 different videogames of the NES catalogue.

The Audio Processing Unit (APU) is not implemented, so the emulator isn't capable of producing any sound yet.

## Screenshots

![Screenshot 1](https://i.imgur.com/qJKEINp.png)

![Screenshot 2](https://imgur.com/HYYGtbK.png)

## Compiling the project
The easiest way to compile the emulator is to use CMake, so that's the first thing that should be installed on your system.

### Linux
First, you'll need to install SFML, as stated in [their website](https://www.sfml-dev.org/index.php), you can do it with the following command:
```bash
  sudo apt-get install libsfml-dev
```

Then you can exectue CMake on the project root directory (it is recommended to create a build directory first):
```bash
  mkdir build
  cd build
  cmake ..
  cmake --build .
```

### Windows
First, you'll need to download the SFML library, you can do so in their [official website](https://www.sfml-dev.org/index.php).

After extracting the .zip, you will get a folder named `SFML-x.x.x`, you can place it on your prefered location on your drive.

Then you can exectue CMake on the project root directory, and call it using the path to the SFML folder as an argument (it is recommended to create a build directory first):
```bash
  mkdir build
  cd build
  cmake  -DSFML_ROOT=C:\Path\to\library\SFML-x.x.x ..
  cmake --build .
```
## Execute the emulator
To execute the emulator, just pass  the ROM (.nes file) as the first argument (it's more simple if you move it to the same place as the emulator):
```bash
  ./NES_Emu <ROM_Name>.nes
```
### Alternative way (Windows only)
If you don't want to use the console to execute the emulator, you can use the Windows file explorer and just drag the ROM file over the emulator executable.

## Optional compilation configurations
There are several arguments you can add to the  `cmake` command when configuring the project.

### Screen Rendering
If you don't want the emulator to generate any window nor render any frame (usually for debugging purposes) you can call cmake with the following argument:
```bash
  cmake -DRENDER_SCREEN=OFF ..
``` 
This argument will compile the emulator **without the SFML library**.

### CPU Log
If you want the emulator to print a log with all the important CPU information (instruction name, cycles executed, value of internal registers...) after executing every instruction, you can call cmake with the following argument:
```bash
  cmake -DPRINT_LOG=ON ..
``` 
It is recommended to only activate this option when `RENDER_SCREEN=OFF`, because the emulator won't be able to perform correctly with both the screen rendering and the log.

## Author
- [Carlos Peñas](https://github.com/carlos-penas)

