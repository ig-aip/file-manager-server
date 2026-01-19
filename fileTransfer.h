#ifndef NETWORK_H
#define NETWORK_H
#include "net.h"
#include "logger.h"
#include "client.h"
#include <memory>
#include <vector>
#include <unordered_map>

class FileReciver
{
    int buffSize = 2048;
    std::unique_ptr<std::vector<char>> buffer_p();
public:
    FileReciver();
};


class FileSender
{
    int buffSize = 2048;
    std::unique_ptr<std::vector<char>> buffer_p();
public:
    FileSender();
};



class FileTransfer : public std::enable_shared_from_this<FileTransfer>
{
    // FileSender sender;
    // FileReciver reciver;
    std::shared_ptr<tcp::socket> socket;
    boost::uuids::uuid uuid;
    Logger& logger;
    std::unordered_map<boost::uuids::uuid, Client> & clients;
    Client& client;

    void sendName(std::string name64Byte);
    void reciveName();
    void sendUUID();
    void receiveClientStatus();
    void sendTcpHeader(Client& acceptedClient);
    void readTcpHeader();

public:
    FileTransfer(std::shared_ptr<tcp::socket> socket, boost::uuids::uuid uuid, Logger& logger,
                 std::unordered_map<boost::uuids::uuid, Client> & clients);

    void startFileSend();
};



#endif // NETWORK_H
