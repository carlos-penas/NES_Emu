#include "notmappedaddressexception.h"

NotMappedAddressException::NotMappedAddressException(Address address, AccessType accessType)
{
    this->address = address;
    this->accessType = accessType;
}
