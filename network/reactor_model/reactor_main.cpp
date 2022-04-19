//
// Created by kongshui on 22-4-18.
//

#include "reactor_server.h"

int main()
{
    if (!ReactorServer::getInstance().init("0.0.0.0", 3333))
        return -1;

    ReactorServer::getInstance().mainLoop();

    return 0;
}