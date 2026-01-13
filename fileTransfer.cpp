#include "fileTransfer.h"



// FileReciver::FileReciver(){
//     buffer_p()->reserve(buffSize);
// }

// FileSender::FileSender(){
//     buffer_p()->reserve(buffSize);
// }


//Transfer
FileTransfer::FileTransfer(std::shared_ptr<tcp::socket> socket, boost::uuids::uuid uuid, Logger& logger):
    socket(socket),
    uuid(uuid),
    logger(logger){
}

void FileTransfer::sendUUID(){
    std::array<char, 16> byteBuffer;
    auto self = shared_from_this();
    std::copy(uuid.begin(), uuid.end(), byteBuffer.begin());
    socket->async_send(boost::asio::buffer(uuid), [self](boost::system::error_code er, std::size_t bytes){
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


void FileTransfer::reciveName(){
    std::string str;
    str.reserve(USERNAME_LENGHT);
    auto self = shared_from_this();
    socket->async_read_some(asio::buffer(str), [self](boost::system::error_code er, std::size_t bytes){
        if(!er){

        }
    });
}



void FileTransfer::startFileSend(){
    sendUUID();
}





