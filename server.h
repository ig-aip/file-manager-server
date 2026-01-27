#ifndef SERVER_H
#define SERVER_H

#include "client.h"
#include "net.h"
#include "logger.h"
#include <unordered_map>
#include <mutex>


class Server : public std::enable_shared_from_this<Server>
{
    asio::io_context ioc;
    tcp::acceptor acceptor;
    std::unordered_map<boost::uuids::uuid,std::shared_ptr<Client>> clients;
    std::mutex clientMutex;

    void start_acceptor();
    boost::uuids::uuid generateUUID() const;

public:
    boost::uuids::uuid addClient(tcp::endpoint endpoint);
    Logger logger;
    Server();
    void start();
};

#endif // SERVER_H
