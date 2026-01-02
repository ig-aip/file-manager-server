#ifndef SERVER_H
#define SERVER_H
#include "net.h"

class Server
{
    asio::io_context& ioc;

public:
    Server(asio::io_context& ioc);
};

#endif // SERVER_H
