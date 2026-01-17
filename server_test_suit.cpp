#include <gtest/gtest.h>
#include "server.h"

TEST(TestSuite, TestName)
{
    Server server{};
    std::thread server_thread(&Server::start, &server);


}
