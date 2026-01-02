#ifndef CLIENT_H
#define CLIENT_H
#include "net.h"
#include <memory.h>

enum class Status{
    waiting,

    sending,
    sending_compleat,

    reciving,
    recive_compleat
};

class Client
{
    Status status;
    //endpoint (ip, port)
    std::shared_ptr<char[64]> username;

public:
    Client();
};

#endif // CLIENT_H
