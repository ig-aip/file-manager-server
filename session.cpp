#include "session.h"
#include "server.h"
#include "iostream"
Session::Session(tcp::socket socket_, Server& server_, Logger& logger_, id::uuid myUUID_):
            socket(std::move(socket_)),
            server(server_),
            logger(logger_),
            myUUID(myUUID_){
    chunk.resize(CHUNK_SIZE);
}




void Session::sendMyUUID(){
    auto self = shared_from_this();

    asio::async_write(socket, asio::buffer(&myUUID, sizeof(myUUID)),
                      [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->readUserName();
        }else {
            self->onDisconnect();
        }
    });
}


void Session::readUserName(){
    auto self = shared_from_this();
    myUserName.resize(64);

    asio::async_read(socket, asio::buffer(myUserName.data(), 64),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->server.registerSession(self);
            self->readClientStatus();

        }else{
            self->onDisconnect();
        }
    });
}

void Session::readClientStatus(){
    auto self = shared_from_this();

    asio::async_read(socket, asio::buffer(&myStatus, sizeof(Status)),
                     [self](boost::system::error_code er, std::size_t bytes){
        std::cout << "STATUS: " << self->myStatus << std::endl;
        if(!er){
            if(self->myStatus == Status::waiting_for_send)
            {
                self->readUUID();

            } else if(self->myStatus == Status::waiting_for_accept)
            {
                self->readClientStatus();

            } else if(self->myStatus == Status::receiving){
                self->sendStatus();

            }
        }
    });
}

void Session::sendStatus()
{
    auto self = shared_from_this();
    asio::async_write(pairSession->getSocket(), asio::buffer(&myStatus, sizeof(myStatus)),
                      [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->transferData();
        }else{
            self->onDisconnect();
        }
    } );
}

void Session::readUUID()
{
    auto self = shared_from_this();
    asio::async_read(socket, asio::buffer(pairUUID.data, sizeof(pairUUID)),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            auto target = self->server.getSession(self->pairUUID);
            if(target){
                self->pairSession = target;

                target->pairSession = self;
                self->sendName(target->myUserName);
            }else{
                self->sendName("N"); // опасно надо уточнить
            }
        }else{
            self->onDisconnect();
        }
    });
}

void Session::transferData()
{

    if(pairSession == nullptr){
        return;
    }


    if(transferedBytes == currrentHeader.file_size_byte){

        std::cout << transferedBytes << " vs " << currrentHeader.file_size_byte << std::endl;
        myStatus = Status::receive_complete;
        readClientStatus();
        return;
    }

    std::size_t remain = currrentHeader.file_size_byte - transferedBytes;


    auto self = shared_from_this();
    size_t sizeToRead = std::min(chunk.size(), chunk.size());

    asio::async_read(pairSession->getSocket(), asio::buffer(chunk.data(), sizeToRead),
                     [self](boost::system::error_code er, std::size_t bytesRead){
        if(!er && bytesRead > 0){
            asio::async_write(self->socket, asio::buffer(self->chunk.data(), bytesRead),
                              [self](boost::system::error_code er1, std::size_t bytesWrite){
                if(!er1){
                    self->transferedBytes += bytesWrite;
                    self->transferData();
                }else{
                    self->onDisconnect();
                }
            });
        }else{
            self->onDisconnect();
        }
    });
}


void Session::sendName(std::string& name64Byte){
    auto self = shared_from_this();

    asio::async_write(socket, asio::buffer(name64Byte.data(), 64),
                      [self](boost::system::error_code er, std::size_t bytes){
                          if(!er){
                              self->readTcpHeader();
                          }else{
                              self->onDisconnect();
                          }
                      });
}

//неправильный uuid
void Session::sendName(std::string&& name64Byte){
    auto self = shared_from_this();
    name64Byte.resize(64);
    asio::async_write(socket, asio::buffer(name64Byte.data(), 64),
                      [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            //если был отправлен непраильный uuid возвращаеися к принятию
            self->readUUID();
        }else{
            self->onDisconnect();
        }
    });
}



void Session::readTcpHeader()
{
    auto self = shared_from_this();

    asio::async_read(socket, asio::buffer(&currrentHeader, sizeof(currrentHeader)),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->sendTcpHeader();
        }else{
            self->onDisconnect();
        }
    });

}




void Session::sendTcpHeader()
{
    auto self = shared_from_this();

    asio::async_write(pairSession->getSocket(), asio::buffer(&currrentHeader, sizeof(currrentHeader)),
                      [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->waitClientResponse();
        }else{
            self->onDisconnect();
        }
    });
}

void Session::waitClientResponse()
{
    //ничего не делаем
    //ждём пока реанимируют с сесии получателя
}

void Session::onDisconnect()
{
    if(socket.is_open()){
        boost::system::error_code er;
        socket.close(er);
        server.removeSession(myUUID);
    }

}

id::uuid Session::getUUID(){
    return myUUID;
}

tcp::socket &Session::getSocket()
{
    return socket;
}

void Session::start()
{
    sendMyUUID();
}
