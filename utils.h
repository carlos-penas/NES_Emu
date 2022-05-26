#ifndef UTILS_H
#define UTILS_H
#include "types.h"

class Utils
{
public:
    static Address joinBytes(Byte msB, Byte lsB) { return (msB << 8) | lsB; }
    static Byte lowByte(Address data) { return (Byte) (data & 0xFF); }
    static Byte highByte(Address data) { return  (Byte) (data >> 8); }
};

#endif // UTILS_H
