#ifndef NET_H
#define NET_H
// #define IP "127.0.0.1"
// #define PORT 55555
// #define USERNAME_LENGHT 64
// #define CHUNK_SIZE 65536
#include<boost/asio.hpp>
#include<boost/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>

const std::string IP = "127.0.0.1";
const int PORT = 55555;
const int USERNAME_LENGHT = 64;
const int CHUNK_SIZE = 65536;

namespace id = boost::uuids;
namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;

#endif // NET_H
