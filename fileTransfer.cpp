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
    client(clients.find(uuid)->second){
}

void FileTransfer::sendUUID(){
    auto self = shared_from_this();
    socket->async_write_some(boost::asio::buffer(uuid), [self](boost::system::error_code er, std::size_t bytes){
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


void FileTransfer::sendTcpHeader(Client& acceptedClient){
    auto self = shared_from_this();
    asio::async_write(*acceptedClient.getSocket(), asio::buffer(&client.getTcpHeader(), sizeof(tcpHeader)), [](boost::system::error_code er, std::size_t bytes){
        //Начать отправлять файл если всё хорошо
    });
}

void FileTransfer::readTcpHeader(){
    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(&client.getTcpHeader(), sizeof(tcpHeader)), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            if(self->client.getStatus() == Status::waiting_for_send){ //если клиент отправляет
                self->sendTcpHeader(self->clients.find(self->client.getTcpHeader().uuid)->second);
            }
            else if(self->client.getStatus() == Status::waiting_for_accept){ //если клиент принимает
                //НАчать принимать файл
            }
        }
    });
}

void FileTransfer::receiveClientStatus(){

    auto self = shared_from_this();
    asio::async_read(*socket, asio::buffer(&client.getStatus(), sizeof(Status)), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
            if(self->client.getStatus() == Status::waiting_for_accept){ //принимающая сторона принимает мета информаци о файле
                self->client.setSocket(self->socket);
                // отправить тспХэдер принимающему клиенту
            }
            else if(self->client.getStatus() == Status::waiting_for_send){ //отправляющая сторона отправляет мета информацию о файле
                self->readTcpHeader();
            }
        }
    });
}


void FileTransfer::reciveName(){
    std::string str;
    str.reserve(USERNAME_LENGHT);
    auto self = shared_from_this();
    socket->async_read_some(asio::buffer(clients.find(uuid)->second.getUsernameLink()), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){
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





