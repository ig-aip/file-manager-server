#include "session.h"
#include "server.h"

Session::Session(tcp::socket socket_, Server& server_, id::uuid myUUID_, ConsoleLogger& logger_):
            socket(std::move(socket_)),
            server(server_),
            myUUID(myUUID_),
            transferedBytes(0),
            logger(logger_){
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
    if(myStatus == Status::receive_complete){
        logger.log("compleate ", "sender(ip: " ,  pairSession->ipS, " port: " ,
                         pairSession->portS, " UUID: ", pairSession->myUUID, " name: ", pairSession->myUserName.erase(pairSession->myUserName.find_last_not_of('\0') + 1),
                         ") reciver(ip: ",ipS, " port: ", portS,
                         " UUID: ",myUUID, " name: ", myUserName.erase(myUserName.find_last_not_of('\0') + 1)," ) file(name: ", currentHeader.fileName, " size: ", currentHeader.file_size_byte, " )");
        resetAllSessions();
    }
    auto self = shared_from_this();

    asio::async_read(socket, asio::buffer(&myStatus, sizeof(Status)),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            if(self->myStatus == Status::waiting_for_send)
            {
                self->readUUID();

            } else if(self->myStatus == Status::waiting_for_accept)
            {
                self->readClientStatus();

            } else if(self->myStatus == Status::receiving){
                self->logger.log("accept ", "sender(ip: " ,  self->pairSession->ipS, " port: " ,
                                 self->pairSession->portS, " UUID: ", self->pairSession->myUUID, " name: ", self->pairSession->myUserName.erase(self->pairSession->myUserName.find_last_not_of('\0') + 1),
                                 ") reciver(ip: ",self->ipS, " port: ", self->portS,
                                 " UUID: ",self->myUUID, " name: ", self->myUserName.erase(self->myUserName.find_last_not_of('\0') + 1)," ) file(name: ", self->currentHeader.fileName, " size: ", self->currentHeader.file_size_byte, " )");
                self->sendStatus();
            }
            else if(self->myStatus == Status::waiting){
                //значит клиент отказался принимать файл
                self->logger.log("reject ", "sender(ip: " ,  self->pairSession->ipS, " port: " ,
                                 self->pairSession->portS, " UUID: ", self->pairSession->myUUID, " name: ", self->pairSession->myUserName.erase(self->pairSession->myUserName.find_last_not_of('\0') + 1),
                                 ") reciver(ip: ",self->ipS, " port: ", self->portS,
                                 " UUID: ",self->myUUID, " name: ", self->myUserName.erase(self->myUserName.find_last_not_of('\0') + 1)," ) file(name: ", self->currentHeader.fileName, " size: ", self->currentHeader.file_size_byte, " )");
                self->sendRejectStatus();
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
            self->logger.log("transfer ", "sender(ip: " ,  self->pairSession->ipS, " port: " ,
                             self->pairSession->portS, " UUID: ", self->pairSession->myUUID, " name: ", self->pairSession->myUserName.erase(self->pairSession->myUserName.find_last_not_of('\0') + 1),
                             ") reciver(ip: ",self->ipS, " port: ", self->portS,
                             " UUID: ",self->myUUID, " name: ", self->myUserName.erase(self->myUserName.find_last_not_of('\0') + 1)," ) file(name: ", self->currentHeader.fileName, " size: ", self->currentHeader.file_size_byte, " )");
            self->transferData();
        }else{
            self->onDisconnect();
        }
    } );
}

void Session::sendRejectStatus(){

    auto self = shared_from_this();
    asio::async_write(pairSession->getSocket(), asio::buffer(&myStatus, sizeof(myStatus)),
                      [self](boost::system::error_code er, std::size_t bytes){
                          if(!er){
                              self->resetAllSessions();
                              self->readClientStatus();
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
                if(target->myStatus == Status::waiting_for_accept){
                    self->pairSession = target;

                    target->pairSession = self;
                    self->sendName(target->myUserName);
                }else{
                    self->sendName("N");
                }

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


    if(transferedBytes == currentHeader.file_size_byte){

        myStatus = Status::receive_complete;
        readClientStatus();
        return;
    }
    if(initPipe()){
        spliceRead();
    }else{
        onDisconnect();
    }

/*
    std::size_t remain = pairSession->currentHeader.file_size_byte - transferedBytes;


    auto self = shared_from_this();
    size_t sizeToRead = std::min(chunk.size(), remain);


    pairSession->getSocket().async_read_some(asio::buffer(chunk.data(), sizeToRead),
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
*/
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
    auto msg = std::make_shared<std::string>(std::move(name64Byte));
    msg->resize(64);
    asio::async_write(socket, asio::buffer(msg->data(), 64),
                      [self, msg](boost::system::error_code er, std::size_t bytes){
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

    asio::async_read(socket, asio::buffer(&currentHeader, sizeof(currentHeader)),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->pairSession->generateMyTcpHeader(self->currentHeader);
            self->sendTcpHeader();
        }else{
            self->onDisconnect();
        }
    });

}




void Session::sendTcpHeader()
{
    auto self = shared_from_this();

    asio::async_write(pairSession->getSocket(), asio::buffer(&pairSession->currentHeader, sizeof(pairSession->currentHeader)),
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
    // -_0
}

void Session::onDisconnect()
{
    if(socket.is_open()){
        closePipe();
        logger.log("disconnect ip: ",ipS, " port: ",portS, " UUID: ", myUUID, " name: ", myUserName);
        boost::system::error_code er;
        socket.close(er);
        server.removeSession(myUUID);
    }

}

void Session::resetAllSessions()
{
    transferedBytes = 0;
    pairSession->myStatus = Status::waiting;
    pairSession->pairSession = nullptr;
    pairSession->pairUUID = id::uuid{};
    pairSession->currentHeader = tcpHeader {};
    pairSession->readClientStatus();
    pairSession = nullptr;
    pairUUID = id::uuid{};
    currentHeader = tcpHeader{};
    myStatus = Status::waiting;
    closePipe();

}

bool Session::initPipe()
{
    if(!pipeInit){
        if(pipe(pipeFds) < 0){
            logger.errLog("failed createPipe");
            return false;
        }

        fcntl(pipeFds[0], F_SETFL, O_NONBLOCK);
        fcntl(pipeFds[1], F_SETFL, O_NONBLOCK);

        fcntl(pipeFds[0], F_SETPIPE_SZ, 1024 * 1024);

        pipeInit = true;
        return true;
    }
    return true;
}

void Session::closePipe()
{
    if(pipeInit){
        close(pipeFds[0]);
        close(pipeFds[1]);
        pipeInit = false;
    }
}

void Session::spliceRead()
{
    auto self = shared_from_this();

    pairSession->socket.async_wait(asio::socket_base::wait_read,
       [self](boost::system::error_code er){
           if(er){
               self->onDisconnect();
               return;
           }

           size_t remain = self->currentHeader.file_size_byte - self->transferedBytes;
           size_t toTransfer = std::min<size_t>(remain, 1024 * 1024);

           int sender = self->pairSession->socket.native_handle();

           ssize_t splicedBytes = splice(
                   sender, NULL,
                   self->pipeFds[1], NULL,
                   toTransfer,
                   SPLICE_F_MOVE | SPLICE_F_NONBLOCK);

           if(splicedBytes > 0){
               self->spliceWrite(splicedBytes);

           }else if(splicedBytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){

               self->spliceRead();

           }else if(splicedBytes == 0){

               self->onDisconnect();
           }else{
               self->logger.errLog("error in splice read: ", strerror(errno));
               self->onDisconnect();
           }

       });
}

void Session::spliceWrite(size_t bytesInPipe)
{
    auto self = shared_from_this();

    socket.async_wait(asio::socket_base::wait_write,
                      [self, bytesInPipe](boost::system::error_code er){
                          if(er){
                              self->onDisconnect();
                              return;
                          }

                          int received = self->socket.native_handle();

                          ssize_t splicedBytes = splice(
                              self->pipeFds[0], NULL,
                              received, NULL,
                              bytesInPipe,
                              SPLICE_F_MOVE | SPLICE_F_NONBLOCK);

                          if(splicedBytes > 0){
                              self->transferedBytes += splicedBytes;

                              if(splicedBytes < bytesInPipe){
                                  self->spliceWrite(bytesInPipe - splicedBytes);
                              }else{
                                  self->transferData();
                              }
                          }else if( splicedBytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)){
                              self->spliceWrite(bytesInPipe);
                          }else{
                              self->logger.errLog("error in bytesInPipe ip: ",self->socket.remote_endpoint().address().to_string(),
                                                  " port: ",self->socket.remote_endpoint().port(), " UUID: ", self->myUUID, " name: ", self->myUserName);
                              self->onDisconnect();
                          }


                      });
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
    ipS = socket.remote_endpoint().address().to_string();
    portS = socket.remote_endpoint().port();

    logger.log("start ip: ", ipS, " port: ", portS,
                " UUID: ", boost::uuids::to_string(myUUID));
    sendMyUUID();
}

Session::~Session()
{
    closePipe();
}




void Session::generateMyTcpHeader(tcpHeader& header){
    std::memcpy(currentHeader.fileName, header.fileName, 225);
    std::memcpy(currentHeader.userName, pairSession->myUserName.c_str(), 64);
    currentHeader.file_size_byte = header.file_size_byte;
    currentHeader.uuid = pairSession->myUUID;

}

