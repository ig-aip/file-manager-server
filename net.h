#ifndef NET_H
#define NET_H
#define IP "25.4.194.225"
#define PORT 55555
#define USERNAME_LENGHT 64
#include<boost/asio.hpp>
#include<boost/uuid.hpp>
#include<boost/uuid/uuid_generators.hpp>
#include<boost/uuid/uuid_io.hpp>



namespace asio = boost::asio;
namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;

#endif // NET_H
