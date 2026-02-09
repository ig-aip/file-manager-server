#ifndef SESSION_H
#define SESSION_H
#include "net.h"
#include "tcpHeader.h"





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


class Server;


class Session : public std::enable_shared_from_this<Session>{
    tcp::socket socket;
    Server& server;
    id::uuid myUUID;
    std::string myUserName;
    Status myStatus;

    id::uuid pairUUID;
    std::shared_ptr<Session> pairSession;

    tcpHeader currentHeader;
    uint64_t transferedBytes = 0;



    std::vector<char> chunk;
    std::array<char, 128> buf;

    void sendMyUUID();
    void readUserName();
    void readClientStatus();
    void readUUID();
    void transferData();

    void sendName(std::string& name64Byte);
    void sendName(std::string&& name64Byte);




    void sendRejectStatus();
    void sendStatus();

    void sendTcpHeader();
    void readTcpHeader();


    void waitClientResponse();

    void onDisconnect();

    void resetAllSessions();


public:
    Session(tcp::socket socket_, Server& server_, id::uuid myUUID_);

    id::uuid getUUID();
    tcp::socket& getSocket();

    void generateMyTcpHeader(tcpHeader& header);

    void restart();

    void start();
};



#endif // SESSION_H
