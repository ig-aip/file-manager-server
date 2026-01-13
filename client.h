#ifndef CLIENT_H
#define CLIENT_H
#include "net.h"
#include <memory>

enum class Status{
    waiting_for_send,
    waiting_for_accept,
    waiting,

    sending,
    sending_compleat,

    reciving,
    recive_compleat
};

class Client
{
    Status status;
    tcp::endpoint endpoint;
    std::string username;

public:
    Client(tcp::endpoint& endpoint);
};

#endif // CLIENT_H
