#include "net.h"
#include <iostream>
#include "server.h"
int main()
{
    auto serv = std::make_shared<Server>();
    serv->start();

}
