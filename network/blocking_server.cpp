//
// Created by kongshui on 2022/4/14.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

int main(int argc, char *argv[])
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        std::cout << "create listen socket error." << std::endl;
        return -1;
    }

    struct sockaddr_in bind_addr;
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(3000);

    if (bind(listen_fd, (struct sockaddr *) &bind_addr, sizeof(bind_addr)) == -1)
    {
        std::cout << "bind listen socket error." << std::endl;
        return -1;
    }

    if (listen(listen_fd, SOMAXCONN) == -1)
    {
        std::cout << "listen error." << std::endl;
        close(listen_fd);
        return -1;
    }

    while (true)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int client_fd = accept(listen_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd != -1)
        {
            //只接受连接，不调用recv收取任何数据
            std::cout << "accept a client connection." << std::endl;
        }

    }

    close(listen_fd);

    return 0;

}