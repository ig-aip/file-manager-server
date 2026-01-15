#include "server.h"
#include <iostream>
#include "fileTransfer.h"
Server::Server() :
    ioc(),
    acceptor(ioc,tcp::endpoint(ip::make_address_v4(IP), PORT)){

}

boost::uuids::uuid Server::generateUUID() const{
    boost::uuids::basic_random_generator<std::mt19937> generate;
    return generate();
}

boost::uuids::uuid Server::addClient(tcp::endpoint endpoint){
    boost::uuids::uuid uuid = generateUUID();
    clients.emplace(uuid, endpoint);
    return uuid;
}

void Server::start(){

    std::cout << (int) Status::receive_complete << std::endl;

    start_acceptor();

    unsigned int threads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> pool;
    pool.reserve(threads);
    auto self = shared_from_this();
    for(int i = 0; i < threads; ++i){
        pool.emplace_back([self]() -> void{
            try{
                self->ioc.run();

            }catch(std::exception& ex){
                self->logger.message(ex.what());
            }
        });
    }

    for(auto& t : pool){ t.join(); }
}

void Server::start_acceptor(){
    std::shared_ptr<tcp::socket> sock = std::make_shared<tcp::socket>(ioc);
    auto self = shared_from_this();
    acceptor.async_accept(*sock,
                          [self, sock](const boost::system::error_code& er){
                              if(!er){

                                  std::cout << "connected" <<std::endl;
                                  auto transfer = std::make_shared<FileTransfer>(sock,
                                                                                 self->addClient(sock->remote_endpoint()),
                                                                                 self->logger,
                                                                                 self->clients);
                                  transfer->startFileSend();

                              }else{
                                  std::stringstream ss;
                                  ss << "error in accept, ip : " << sock->remote_endpoint().address().to_string()
                                     << " port: " << sock->remote_endpoint().port()
                                     << "error: " << er.message();
                                  self->logger.message(ss.str());
                              }

                              self->start_acceptor();

                          });
}
