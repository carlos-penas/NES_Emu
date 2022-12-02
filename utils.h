#ifndef UTILS_H
#define UTILS_H
#include "types.h"
#include <QString>

class Utils
{
public:
    static Address joinBytes(Byte msB, Byte lsB) { return (msB << 8) | lsB;}
    static Byte joinBits(Byte msb, Byte lsb) { return (msb << 1) | lsb;}
    static Byte lowByte(Address data) { return (Byte) (data & 0xFF);}
    static Byte highByte(Address data) { return  (Byte) (data >> 8);}
    static QString hexString(Byte number) {return QString("%1").arg(number,2,16,QLatin1Char('0')).toUpper();};
    static QString hexString16(uint16_t number){return QString("%1").arg(number,4,16,QLatin1Char('0')).toUpper();};
};

#endif // UTILS_H
