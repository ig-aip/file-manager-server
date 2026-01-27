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
                           std::unordered_map<boost::uuids::uuid, std::shared_ptr<Client>> & clients):
    socket(socket),
    uuid(uuid),
    logger(logger),
    clients(clients),
    client(clients.find(uuid)->second)
{
    client->setUUID(uuid);
    chunk.resize(CHUNK_SIZE);
}

void FileTransfer::receiveUUID(){
    auto acUUID = std::make_shared<boost::uuids::uuid>();

    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(acUUID->data(), acUUID->size()), [self, acUUID](boost::system::error_code er, std::size_t bytes){
        //std::cout << boost::uuids::to_string(*acUUID) << std::endl;
        std::cout << *acUUID <<std::endl;
        if(!er){
            std::shared_ptr<Client> targetClient = nullptr;
            {
                std::lock_guard<std::mutex> lock(self->mtx);
                auto it = self->clients.find(*acUUID);
                if(it != self->clients.end()){
                    targetClient = it->second;
                }
            }

            std::cout << " NEEDED UUID: " << *acUUID << " bytes: "<< bytes << "uuid size: " << acUUID->size()<<std::endl;

            if(targetClient)
            {
                std::cout <<"FINDED : "<< targetClient->getUsernameLink() << "UUID: " << *targetClient->getUUIDp() << std::endl;
                //если uuid и paiClient совпадаютЮ значит с targetCLient никто не связан
                if(targetClient->getPairClient() == *targetClient->getUUIDp()){
                    targetClient->setPairClient(*self->client->getUUIDp());
                    self->client->setPairClient(*targetClient->getUUIDp());
                    self->sendName(targetClient->getUsernameLink());

                }else{
                    // кто то уже занял принимающий сокет
                    std::cout << "NOT FREE" << std::endl;
                }

            }else{
                std::string sendName{"N"};
                sendName.resize(64);
                self->sendName(sendName);
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
    socket->async_write_some(boost::asio::buffer(client->getUUIDp()->data(), client->getUUIDp()->size()), [self](boost::system::error_code er, std::size_t bytes){
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


void FileTransfer::sendTcpHeader(std::shared_ptr<Client> acceptedClient){
    auto self = shared_from_this();
    asio::async_write(*acceptedClient->getSocket(), asio::buffer(&client->getTcpHeader(), sizeof(tcpHeader)), [self](boost::system::error_code er, std::size_t bytes){
        self->receiveClientStatus(); // надо узнать, согласен ликлент на принятие файла
    });
}

void FileTransfer::readTcpHeader(){
    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(&client->getTcpHeader(), sizeof(tcpHeader)), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            if(self->client->getStatus() == Status::waiting_for_send){ //если клиент отправляет
                //отпарвляем мета инфу о том, какое имя у файла, какой username и uuid

                std::lock_guard<std::mutex> lock(self->mtx);
                auto it = self->clients.find(self->client->getPairClient());
                if(it != self->clients.end()){
                    self->sendTcpHeader(it->second);
                }else{
                    //потеря соединение с другой стороной
                }

            }
            // else if(self->client.getStatus() == Status::waiting_for_accept){ //если клиент принимает
            //     //НАчать принимать файл
            // }
        }
    });
}

void FileTransfer::sendFileFromAccept(){
    //если есть какая то связь
    if(client->getPairClient() != *client->getUUIDp()){
        auto self = shared_from_this();
        std::lock_guard<std::mutex> lock(mtx);
        clients.find(client->getPairClient())->second->getSocket()->async_read_some(asio::buffer(chunk.data(), chunk.size()),
                                                                                    [self](boost::system::error_code er, std::size_t bytesRead){
            if(bytesRead == 0){
                std::cout <<"compleated " << std::endl;
                return;
            }
            if(!er){
                asio::async_write(*self->client->getSocket(),asio::buffer(self->chunk.data(), bytesRead), [self](boost::system::error_code er1, std::size_t bytesWrite){
                    if(!er1){
                        self->sendFileFromAccept();
                    }else{
                        std::cout << "file trader " <<er1.what() << std::endl;
                        return;
                    }
                });
            }else{
                std::cout << "file trader " <<er.what() << std::endl;
                return;
            }
        });
    }


}

void FileTransfer::receiveClientStatus(){
// читаем и проверяем, клиент принимает или отправляет файлы по статусу
    auto self = shared_from_this();
    auto ar = std::make_shared<std::array<char, 2>>();
    asio::async_read(*socket, asio::buffer(ar->data(), ar->size()), [self](boost::system::error_code er, std::size_t bytes){
        //self->client->getStatus() = (Status)((int)self->client->getStatus());
        std::cout << "ar is here" << std::endl;
        std::cout <<"bytes: "<< bytes << "status: "<< (int) self->client->getStatus() << std::endl;
        if(!er){

            if(self->client->getStatus() == Status::waiting_for_accept){ //принимающая сторона
                self->client->setSocket(self->socket);
                std::cout << "ACCEPT : " << self->client->getUsernameLink() << std::endl;
                self ->receiveClientStatus(); //получаем Status::reciveing для начала отправления файла
                //  принимающий клиент заносит свой сокет в unordered_map, позже на этот сокет будет отправлена информация
            }
            else if(self->client->getStatus() == Status::receiving){
                if(self->client->getPairClient() != *self->client->getUUIDp()){
                    self->sendFileFromAccept();
                    std::cout << "RECIVE : " << self->client->getUsernameLink() << std::endl;
                }
            }
            else if(self->client->getStatus() == Status::waiting_for_send || bytes == 2){ //отправляющая сторона отправляет мета информацию о файле нам на сервер
                std::cout << "SENDING : " << self->client->getUsernameLink() << std::endl;
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
            std::cout << "name: "  << '\n';
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
    socket->async_read_some(asio::buffer(client->getUsernameLink()),
                     [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            std::cout << "Name: " << self->client->getUsernameLink()  << " bytes: " << bytes  << std::endl;
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





