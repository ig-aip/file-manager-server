#include "client.h"

Client::Client(tcp::endpoint& endpoint) :
    endpoint(endpoint) {
}


std::string& Client::getUsernameLink(){
    return username;
}

tcpHeader& Client::getTcpHeader(){
    return header;
}


void Client::setSocket(std::shared_ptr<tcp::socket>  socketNew){
    socket = socketNew;
}
std::shared_ptr<tcp::socket> Client::getSocket(){
    return socket;
}

void Client::setWaitingForSend()
{
    status = Status::waiting_for_send;
}

void Client::setWaitingForAccept()
{
    status = Status::waiting_for_accept;
}

void Client::setWaiting()
{
    status = Status::waiting;
}


void Client::setSending()
{
    status = Status::sending;
}

void Client::setSendingComplete()
{
    status = Status::sending_complete;
}


void Client::setReceiving()
{
    status = Status::receiving;
}

void Client::setReceiveComplete()
{
    status = Status::receive_complete;
}


Status& Client::getStatus()
{
    return status;
}
