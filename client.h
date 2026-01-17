#ifndef CLIENT_H
#define CLIENT_H
#include "net.h"
#include "tcpHeader.h"


enum class Status{
    waiting_for_send,
    waiting_for_accept,
    waiting,

    sending,
    sending_complete,

    receiving,
    receive_complete
};

class Client
{
    Status status;
    tcp::endpoint endpoint;
    std::string username;
    tcpHeader header;
    std::shared_ptr<tcp::socket> socket;

public:
    Client(tcp::endpoint& endpoint);

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
