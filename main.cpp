#include "server.h"

int main()
{
    auto serv = std::make_shared<Server>();
    serv->start();
    return 1;
}
