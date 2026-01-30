#ifndef SERVER_H
#define SERVER_H


#include "net.h"
#include "logger.h"
#include <unordered_map>
#include <mutex>
#include "session.h"

class Server : public std::enable_shared_from_this<Server>
{
    asio::io_context ioc;
    tcp::acceptor acceptor;
    std::unordered_map<id::uuid, std::shared_ptr<Session>> sessions;
    std::mutex clientMutex;

    void start_acceptor();
    boost::uuids::uuid generateUUID() const;


public:

    boost::uuids::uuid addClient(tcp::endpoint endpoint);
    Logger logger;
    Server();
    void start();

    void registerSession(std::shared_ptr<Session> newSession);
    void removeSession(id::uuid deleteUUID);
    std::shared_ptr<Session> getSession(id::uuid getUUID);
};

#endif // SERVER_H
