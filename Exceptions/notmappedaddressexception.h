#ifndef NOTMAPPEDADDRESSEXCEPTION_H
#define NOTMAPPEDADDRESSEXCEPTION_H
#include <exception>
#include <cstring>
#include <sstream>
#include "compilationSettings.h"

#ifdef COMPILE_WINDOWS
#include "../types.h"
#else
#include "types.h"
#endif

enum AccessType{
    ReadAttempt,
    WriteAttempt
};


using namespace std;

class NotMappedAddressException : public exception
{
public:
    NotMappedAddressException(Address address, AccessType accessType);
    const char * what () const throw ()
    {
        stringstream ss;
        if(accessType == ReadAttempt)
            ss << "Attempting to read from an invalid address: 0x" << hex << address;
        else
            ss << "Attempting to write to an invalid address: 0x" << hex << address;
        char * str = new char[60];
#ifdef COMPILE_WINDOWS
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
