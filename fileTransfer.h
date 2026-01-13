#ifndef NETWORK_H
#define NETWORK_H
#include "net.h"
#include "logger.h"
#include <memory>
#include <vector>

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

    void reciveName();
    void sendUUID();

public:
    FileTransfer(std::shared_ptr<tcp::socket> socket, boost::uuids::uuid uuid, Logger& logger);
    void startFileSend();
};



#endif // NETWORK_H
