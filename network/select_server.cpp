//
// Created by kongshui on 2022/4/13.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <errno.h>

//自定义代表无效fd的值
#define INVALID_FD -1

int main(int argc, char* argv[])
{
    //创建一个侦听socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        std::cout << "create listen socket error." << std::endl;
        return -1;
    }

    std::cout << "listenfd: " << listenfd << std::endl;

    //初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(3000);
    if (bind(listenfd, (struct sockaddr *) &bindaddr, sizeof(bindaddr)) == -1)
    {
        std::cout << "bind listen socket error." << std::endl;
        close(listenfd);
        return -1;
    }

    if(listen(listenfd, SOMAXCONN) == -1)
    {
        std::cout << "listen error." << std::endl;
        close(listenfd);
        return -1;
    }

    std::vector<int> clientfds;
    clientfds.clear();
    int maxfd = listenfd;

    std::cout << "maxfd: " << maxfd << std::endl;

    while(true)
    {
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(listenfd, &readset);

        int clientfdslength = clientfds.size();
        for (int i = 0; i < clientfdslength; ++i)
        {
            if (clientfds[i] != INVALID_FD)
            {
                FD_SET(clientfds[i], &readset);
            }
        }

        timeval tm;
        tm.tv_sec = 1;
        tm.tv_usec = 0;

        int ret = select(maxfd + 1, &readset, NULL, NULL, &tm);
        if (ret == -1)
        {
            //出错，退出程序。
            if (errno != EINTR)
                break;
        }
        else if (ret == 0)
        {
            //select 函数超时，下次继续
            continue;
        }else
        {
            if (FD_ISSET(listenfd, &readset))
            {
                //侦听socket的可读事件，则表明有新的连接到来
                struct sockaddr_in clientaddr;
                socklen_t clientaddrlen = sizeof(clientaddr);

                int clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientaddrlen);
                if (clientfd == -1)
                {
                    //接受连接出错，退出程序
                    break;
                }

                //只接受连接，不调用recv收取任何数据
                std:: cout << "accept a client connection, fd: " << clientfd << std::endl;
                clientfds.push_back(clientfd);
                //记录一下最新的最大fd值，以便作为下一轮循环中select的第一个参数
                if (clientfd > maxfd)
                    maxfd = clientfd;
            }else
            {
                char recvbuf[64];
                int clientfdslength = clientfds.size();

                for (int i = 0; i < clientfdslength; ++i)
                {
                    if (clientfds[i] != -1 && FD_ISSET(clientfds[i], &readset))
                    {
                        memset(recvbuf, 0, sizeof(recvbuf));
                        //非侦听socket，则接收数据
                        int length = recv(clientfds[i], recvbuf, 64, 0);
                        if (length <= 0 && errno != EINTR)
                        {
                            //收取数据出错了
                            std::cout << "recv data error, clientfd: " << clientfds[i] << std::endl;
                            close(clientfds[i]);
                            //不直接删除该元素，将该位置的元素置位-1
                            clientfds[i] = INVALID_FD;
                            continue;
                        }

                        std::cout << "clientfd: " << clientfds[i] << ", recv data: " << recvbuf << std::endl;
                    }
                }
            }
        }
    }

    //关闭所有客户端socket
    int clientfdslength = clientfds.size();
    for (int i = 0; i < clientfdslength; ++i)
    {
        if (clientfds[i] != INVALID_FD)
        {
            close(clientfds[i]);
        }
    }

    //关闭侦听socket
    close(listenfd);

    return 0;
}