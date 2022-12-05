#ifndef UTILS_H
#define UTILS_H
#include "types.h"
#include <iostream>
#include <sstream>
#include <iomanip>

class Utils
{
public:
    static Address joinBytes(Byte msB, Byte lsB) { return (msB << 8) | lsB;}
    static Byte joinBits(Byte msb, Byte lsb) { return (msb << 1) | lsb;}
    static Byte lowByte(Address data) { return (Byte) (data & 0xFF);}
    static Byte highByte(Address data) { return  (Byte) (data >> 8);}
    static std::string hexString(Byte number)
    {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << unsigned(number);
        return ss.str();
    };
    static std::string hexString16(uint16_t number)
    {
        std::string s = "";
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << unsigned(number);
        return s + ss.str();
    };
};

#endif // UTILS_H
