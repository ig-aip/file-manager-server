#include <gtest/gtest.h>
#include "server.h"



class TestClient{
    public:
    asio::io_context ioc;
    id::uuid UUID;
    tcp::socket socket;

    TestClient() : socket(ioc) {}

    void connectToServer(){
        socket.connect(tcp::endpoint{ip::make_address_v4(IP), PORT});
    }


    void readUUID(){
        boost::system::error_code er;
        asio::read(socket, asio::buffer(&UUID, sizeof(UUID)), er);
        ASSERT_FALSE(er) << "failed to read UUID from server";
    }

    void sendName(std::string name){
        name.resize(64);
        boost::system::error_code er;
        asio::write(socket, asio::buffer(&name, 64), er);
        ASSERT_FALSE(er);
    }

    void sendStatus(Status status){
        boost::system::error_code er;
        asio::write(socket, asio::buffer(&status, sizeof(status)), er);
        ASSERT_FALSE(er);
    }

    void sendUUID(id::uuid targetUUID){
        boost::system::error_code er;
        asio::write(socket, asio::buffer(&targetUUID, sizeof(targetUUID)), er);
        ASSERT_FALSE(er);
    }

    std::string readName(){
        boost::system::error_code er;
        std::vector<char> buf(64);
        asio::read(socket, asio::buffer(buf.data(), 64), er);
        return std::string(buf.data(), 64);
    }


    void sendTcpHeader(const tcpHeader& header){
        boost::system::error_code er;
        asio::write(socket, asio::buffer(&header, sizeof(header)), er);
        ASSERT_FALSE(er);
    }

    tcpHeader readTcpHeader(){
        tcpHeader header;
        asio::read(socket, asio::buffer(&header, sizeof(header)));
        return header;
    }


    Status readStatus(){
        Status status;
        asio::read(socket, asio::buffer(&status, sizeof(status)));
        return status;
    }

    void sendData(const std::vector<char>& data){
        boost::system::error_code er;
        asio::write(socket, asio::buffer(&data, sizeof(data)), er);
        ASSERT_FALSE(er);
    }


    void readData(std::vector<char>& buf_out){
        asio::read(socket, asio::buffer(&buf_out, sizeof(buf_out)));
    }


    void close(){
        socket.close();
    }
};




class ServerTest : public ::testing::Test{
public:
    std::shared_ptr<Server> server;
    std::thread serverThread;
    std::atomic<bool> stopServer{false};

    void SetUp() override{
        server = std::make_shared<Server>();

        serverThread = std::thread([this](){
            server->start();
        });

        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }


    void TearDown() override{
        if(server){
            server->stop();
        }

        if(serverThread.joinable()){
            serverThread.join();
        }
    }
};



TEST_F(ServerTest, SendInvalidUUID){
    TestClient sender;
    sender.connectToServer();
    sender.readUUID();
    sender.sendName("Test");


    sender.sendStatus(Status::waiting_for_send);

    id::random_generator gen;
    id::uuid fakeUUID = gen();

    sender.sendUUID(fakeUUID);

    auto response =  sender.readName();

    ASSERT_GE(response.size(), 1);
    ASSERT_EQ(response[0], 'N');
}



TEST_F(ServerTest, ConnectionAndMessages)
{
    TestClient client;
    ASSERT_NO_THROW(client.connectToServer());

    client.readUUID();
    ASSERT_FALSE(client.UUID.is_nil());

    std::cout << client.UUID << std::endl;
    std::string name = "test";
    client.sendName(name);



    auto session = server->getSession(client.UUID);
    ASSERT_NE(session, nullptr);

}
















