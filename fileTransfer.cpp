#include "fileTransfer.h"
#include "iostream"


// FileReciver::FileReciver(){
//     buffer_p()->reserve(buffSize);
// }

// FileSender::FileSender(){
//     buffer_p()->reserve(buffSize);
// }


//Transfer
FileTransfer::FileTransfer(std::shared_ptr<tcp::socket> socket, boost::uuids::uuid uuid, Logger& logger,
                           std::unordered_map<boost::uuids::uuid, Client> & clients):
    socket(socket),
    uuid(uuid),
    logger(logger),
    clients(clients),
    client(clients.find(uuid)->second)
{
    client.setUUID(uuid);
    chunk.resize(CHUNK_SIZE);
}

void FileTransfer::receiveUUID(){
    auto acUUID = std::make_shared<boost::uuids::uuid>();
    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(*acUUID), [self, acUUID](boost::system::error_code er, std::size_t bytes){
        std::cout << boost::uuids::to_string(*acUUID);
        if(!er){
            //если есть то поинтер возьмётся, если нету то возьмется nullptr
            self->client.setPairClient(self->clients.find(*acUUID) != self->clients.end() ? &self->clients.find(*acUUID)->second :
                                           nullptr);


            if(self->client.getPairClient()){
                if(self->client.getPairClient()->getPairClient() == nullptr){ // если принимающие клиент никого не принимает
                    self->client.getPairClient()->setPairClient(&self->client);
                    self->sendName(self->client.getPairClient()->getUsernameLink());
                    std::cout << self->client.getPairClient()->getUsernameLink() << std::endl;
                }else{
                    //кого то принимает
                }
            }else {
                std::string sendName{"N"};
                sendName.resize(64);
                self->sendName(sendName);
                self->receiveUUID();
            }


        }else{
            std::stringstream ss;
            ss << "error in receive uuid from ip: " << self->socket->remote_endpoint().address()
               << ", PORT: " << self->socket->remote_endpoint().port()
               << ", Error: " <<er.what();
            self->logger.message(ss.str());
        }
    });
}

void FileTransfer::sendUUID(){
    auto self = shared_from_this();
    socket->async_write_some(boost::asio::buffer(*client.getUUIDp()), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            self->reciveName();
        }else{
            std::stringstream ss;
            ss << "error in send UUID, error: " << er.message() << "....RETRYING....";
            self->logger.message(ss.str());
            self->sendUUID();
        }
    });
}


void FileTransfer::sendTcpHeader(Client* acceptedClient){
    auto self = shared_from_this();
    asio::async_write(*acceptedClient->getSocket(), asio::buffer(&client.getTcpHeader(), sizeof(tcpHeader)), [self](boost::system::error_code er, std::size_t bytes){
        self->receiveClientStatus(); // надо узнать, согласен ликлент на принятие файла
    });
}

void FileTransfer::readTcpHeader(){
    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(&client.getTcpHeader(), sizeof(tcpHeader)), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            if(self->client.getStatus() == Status::waiting_for_send){ //если клиент отправляет
                //отпарвляем мета инфу о том, какое имя у файла, какой username и uuid
                //self->sendTcpHeader(self->clients.find(self->client.getTcpHeader().uuid)->second);
                //если клиент существет то возьмётся указатель, если нет, то нулптр
                self->sendTcpHeader(self->client.getPairClient());
            }
            // else if(self->client.getStatus() == Status::waiting_for_accept){ //если клиент принимает
            //     //НАчать принимать файл
            // }
        }
    });
}

void FileTransfer::sendFileFromAccept(){
    if(client.getPairClient() != nullptr){
        auto self = shared_from_this();
        asio::async_read(*client.getPairClient()->getSocket(), asio::buffer(chunk), [self](boost::system::error_code er, std::size_t bytes){
            std::cout << "bytes: " << bytes << std::endl;
            asio::async_write(*self->client.getSocket(), asio::buffer(self->chunk), [self](boost::system::error_code er, std::size_t bytes){
                self->sendFileFromAccept();
            });
        });
    }


}

void FileTransfer::receiveClientStatus(){
// читаем и проверяем, клиент принимает или отправляет файлы по статусу
    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(&client.getStatus(), sizeof(Status)), [self](boost::system::error_code er, std::size_t bytes){
        self->client.getStatus() = (Status)((int)self->client.getStatus());
        std::cout <<"bytes: "<< bytes << "status: "<< (int) self->client.getStatus() << std::endl;
        if(!er){

            if(self->client.getStatus() == Status::waiting_for_accept){ //принимающая сторона
                self->client.setSocket(self->socket);
                std::cout << "ACCEPT : " << self->client.getUsernameLink() << std::endl;
                self ->receiveClientStatus(); //получаем Status::reciveing для начала отправления файла
                //  принимающий клиент заносит свой сокет в unordered_map, позже на этот сокет будет отправлена информация
            }
            else if(self->client.getStatus() == Status::receiving){
                if(self->client.getPairClient() != nullptr){
                    self->sendFileFromAccept();
                    std::cout << "RECIVE : " << self->client.getUsernameLink() << std::endl;
                }
            }
            else if(self->client.getStatus() == Status::waiting_for_send){ //отправляющая сторона отправляет мета информацию о файле нам на сервер
                std::cout << "SENDING : " << self->client.getUsernameLink() << std::endl;
                self->receiveUUID();
            }
        }

    });
}
//отправлять 64 байта
void FileTransfer::sendName(std::string name64Byte){
    auto self = shared_from_this();
    asio::async_write(*socket, asio::buffer(name64Byte), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            std::cout << "name: " << self->client.getPairClient()->getUsernameLink().size() << '\n';
            self->readTcpHeader();
        } else{
            std::stringstream ss;
            ss << "error in send 64 byte Name from ip: " << self->socket->remote_endpoint().address()
               << ", PORT: " << self->socket->remote_endpoint().port()
               << ", Error: " <<er.message();
            self->logger.message(ss.str());
        }
    });
}


void FileTransfer::reciveName(){

    auto self = shared_from_this();
    socket->async_read_some(asio::buffer(client.getUsernameLink()),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            std::cout << "Name: " << self->client.getUsernameLink()  << " bytes: " << bytes  << std::endl;
            self->receiveClientStatus();
        }else{
            std::stringstream ss;
            ss << "error in accept from ip: " << self->socket->remote_endpoint().address()
               << ", PORT: " << self->socket->remote_endpoint().port()
               << ", Error: " <<er.message();
            self->logger.message(ss.str());
        }
    });
}



void FileTransfer::startFileSend(){
    sendUUID();
}





