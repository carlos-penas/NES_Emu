#ifndef NOTMAPPEDADDRESSEXCEPTION_H
#define NOTMAPPEDADDRESSEXCEPTION_H
#include <exception>
#include <types.h>
#include <cstring>
#include <sstream>

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
        strcpy(str,ss.str().c_str());
        return str;
    };

private:
    Address address;
    AccessType accessType;

};

#endif // NOTMAPPEDADDRESSEXCEPTION_H
