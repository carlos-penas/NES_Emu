#ifndef TYPES_H
#define TYPES_H
#include <cstdint>

typedef uint8_t Register8;
typedef uint16_t Register16;

typedef uint8_t Byte;
typedef uint16_t Address;

namespace PPURegisters {

    union AddressRegister{
        struct{
            Register16 tileOffsetX : 5;
            Register16 tileOffsetY : 5;
            Register16 nametableX : 1;
            Register16 nametableY : 1;
            Register16 pixelOffsetY : 3;
            Register16 unused : 1;
        };
        Register16  value;

        struct{
            Register8 LByte;
            Register8 HByte;
        };
    };

    union ControlRegister{
        struct{
            Register8 nametableX : 1;
            Register8 nametableY : 1;
            Register8 VRAMincrement : 1;
            Register8 spriteAdress : 1;
            Register8 backgroundAddress : 1;
            Register8 spriteSize : 1;
            Register8 MstSlvMode : 1;
            Register8 generateNMI : 1;
        };
        Register8 value;
    };

    union StatusRegister{
        struct{
            Register8 unused : 5;
            Register8 spriteOverflow : 1;
            Register8 spriteZeroHit : 1;
            Register8 VBlank : 1;
        };
        Register8 value;
    };

    union MaskRegister{
        struct{
            Register8 greyScale : 1;
            Register8 showLeftBackground : 1;
            Register8 showLeftSprites : 1;
            Register8 renderBackground : 1;
            Register8 renderSprites : 1;
            Register8 emphasizeRed : 1;
            Register8 emphasizeGreen : 1;
            Register8 emphasizeBlue : 1;
        };
        Register8 value;
    };

    struct ShiftRegister16{
        Register16 value;
        void shiftLeft(){value <<= 1;};
        void shiftRight(){value >>= 1;};
    };

    struct ShiftRegister8{
        Register8 value;
        void shiftLeft(){value <<= 1;};
        void shiftRight(){value >>= 1;};
    };
}



#endif // TYPES_H
