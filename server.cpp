#include "server.h"
#include <iostream>
#include "fileTransfer.h"
Server::Server() :
    ioc(),
    acceptor(ioc){

    boost::system::error_code er;
    acceptor.open(tcp::endpoint{ip::make_address_v4(IP), PORT}.protocol(), er);
    if(er){
        logger.message(std::string{"cant open acceptor, Error: "}.append(er.what()));
        return;
    }

    acceptor.set_option(asio::socket_base::reuse_address(true), er);
    if(er){
        logger.message(std::string{"cant set reuse address true, Error: "}.append(er.what()));
        return;
    }

    acceptor.bind(tcp::endpoint{ip::make_address_v4(IP), PORT}, er);
    if(er){
        logger.message(std::string{"cant bind acceptor, Error: "}.append(er.what()));
        return;
    }


    acceptor.listen(asio::socket_base::max_listen_connections, er);
    if(er){
        logger.message(std::string{"cant listen acceptor, Error: "}.append(er.what()));
        return;
    }

}

boost::uuids::uuid Server::generateUUID() const{
    boost::uuids::basic_random_generator<std::mt19937> generate;
    return generate();
}

boost::uuids::uuid Server::addClient(tcp::endpoint endpoint){
    boost::uuids::uuid uuid = generateUUID();
    auto newClient = std::make_shared<Client>(endpoint);
    newClient->setPairClient(uuid);

    std::lock_guard<std::mutex> lock(clientMutex);
    clients[uuid] = newClient;
    return uuid;
}

void Server::start(){


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
                self->logger.message(std::string{"error in threads io_context run: "}.append(ex.what()));
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
                                                                                 self->clients,
                                                                                 self->clientMutex);
                                  transfer->startFileSend();

                              }else{
                                  self->logger.message(std::string{"err in accept, error: "}.append(er.what()));

                              }

                              self->start_acceptor();

                          });
}
