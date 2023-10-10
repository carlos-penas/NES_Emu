#ifndef NOTMAPPEDADDRESSEXCEPTION_H
#define NOTMAPPEDADDRESSEXCEPTION_H
#include <exception>
#include <cstring>
#include <sstream>

#include "../types.h"

enum AccessType{
    ReadAttempt,
    WriteAttempt
};

class NotMappedAddressException : public std::exception
{
public:
    NotMappedAddressException(Address address, AccessType accessType);
    const char * what () const throw ()
    {
        std::stringstream ss;
        if(accessType == ReadAttempt)
            ss << "Attempting to read from an invalid address: 0x" << std::hex << address;
        else
            ss << "Attempting to write to an invalid address: 0x" << std::hex << address;
        char * str = new char[60];
#ifdef _WIN32
        strcpy_s(str,ss.str().length(), ss.str().c_str());
#else
        strcpy(str,ss.str().c_str());
#endif
        return str;
    };

private:
    Address address;
    AccessType accessType;

};

#endif // NOTMAPPEDADDRESSEXCEPTION_H
