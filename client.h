#ifndef CLIENT_H
#define CLIENT_H
#include "net.h"
#include <memory>

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

public:
    Client(tcp::endpoint& endpoint);

    std::string& getUsernameLink();

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
