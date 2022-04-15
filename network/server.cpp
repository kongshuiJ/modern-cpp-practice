//
// Created by kongshui on 2022/4/12.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <vector>

int main(int argc, char *argv[])
{
    // 1. create listen socker
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        printf("create listen socket error.\n");
        return -1;
    }

    // 2. init server
    struct sockaddr_in bindaddr{};
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(3002);

    if (bind(listenfd, (struct sockaddr *) &bindaddr, sizeof(bindaddr)))
    {
        printf("bind listen socker error.\n");
        return -1;
    }

    // 3. start listen
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        printf("listen error.\n");
        return -1;
    }

    // record all client
    std::vector<int> clientfds;
    while (true)
    {
        struct sockaddr_in clientaddr;
        socklen_t clientaddrlen = sizeof(clientaddr);
        // 4. accept client
        int clientfd = accept(listenfd, (struct sockaddr *) &clientaddr, &clientaddrlen);
        if (clientfd != -1)
        {
            char recvBuf[32] = {0};
            // 5. recv client's data
            int ret = recv(clientfd, recvBuf, 32, 0);
            if (ret > 0)
            {
                std::cout << "recv data from client, data: " << recvBuf << std::endl;
                //6. 将收到的数据原封不动地发给客户端
                ret = send(clientfd, recvBuf, strlen(recvBuf), 0);
                if (ret != strlen(recvBuf))
                    std::cout << "send data error." << std::endl;

                std::cout << "send data to client successfully, data: " << recvBuf << std::endl;
            }
            else
            {
                std::cout << "recv data error." << std::endl;
            }

            // close(clientfd)
            clientfds.push_back(clientfd);
        }
    }

    // 7. close socket
    close(listenfd);

    return 0;
}