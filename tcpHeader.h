#ifndef TCPHEADER_H
#define TCPHEADER_H
#include "net.h"

#pragma pack(push, 1)

struct tcpHeader{
    boost::uuids::uuid uuid;
    char userName[64];
    char fileName[225];
    uint64_t file_size_byte;
};

#pragma pack(pop)



#endif // TCPHEADER_H
