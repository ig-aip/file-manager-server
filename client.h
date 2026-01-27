#ifndef CLIENT_H
#define CLIENT_H
#include "net.h"
#include "tcpHeader.h"

#pragma pack(push, 1)
enum Status : uint8_t {
    waiting_for_send = 0,
    waiting_for_accept = 1,
    waiting = 2,

    sending = 3,
    sending_complete = 4,

    receiving = 5,
    receive_complete = 6,

    Connected = 7,
    Disconnected = 8
};

#pragma pack(pop)

class Client
{
    Status status;
    tcp::endpoint endpoint;
    std::string username;
    tcpHeader header;
    std::shared_ptr<tcp::socket> socket;
    boost::uuids::uuid uuid;
    boost::uuids::uuid pairClient;

public:
    Client(tcp::endpoint& endpoint);

    void setPairClient(boost::uuids::uuid pClient);
    boost::uuids::uuid getPairClient();

    void setUUID(boost::uuids::uuid uuid);
    boost::uuids::uuid* getUUIDp();

    std::string& getUsernameLink();
    tcpHeader& getTcpHeader();

    void setSocket(std::shared_ptr<tcp::socket> socket_);
    std::shared_ptr<tcp::socket> getSocket();


    // waiting
    void setWaitingForSend();
    void setWaitingForAccept();
    void setWaiting();

    // sending
    void setSending();
    void setSendingComplete();

    // receiving
    void setReceiving();
    void setReceiveComplete();

    Status& getStatus();
};

#endif // CLIENT_H
