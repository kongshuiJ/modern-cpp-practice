//
// Created by kongshui on 2022/4/13.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_PROT     3005
#define SEND_DATA       "helloworld"

int main(int argc, char *argv[])
{
    // 1. create socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1)
    {
        printf("create client socket error.\n");
        return -1;
    }

    // 连接成功以后，我们再将 client_fd 设置成非阻塞模式，
    // 不能在创建时就设置，这样会影响到 connect 函数的行为
    int old_socket_flag = fcntl(client_fd, F_GETFL, 0);
    int new_socket_flag = old_socket_flag | O_NONBLOCK;

    if (fcntl(client_fd, F_SETFL, new_socket_flag) == -1)
    {
        close(client_fd);
        printf("set socket to nonblock error.\n");
        return -1;
    }

    // 2. connect server
    struct sockaddr_in server_addr;
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_port        = htons(SERVER_PROT);

    for(;;)
    {
        int ret = connect(client_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
        if (ret == 0)
        {
            printf("connect to server successfully.\n");
            close(client_fd);
            return 0;
        } else if (ret == -1)
        {
            if (errno == EINTR)
            {
                // connect 动作被信号中断，重试connect
                printf("connecting interruptted by signal, try again.\n");
                continue;
            } else if (errno == EINPROGRESS)
            {
                // 连接正在尝试中
                break;
            } else
            {
                // real error
                close(client_fd);
                return -1;
            }
        }
    }

    fd_set write_set;
    FD_ZERO(&write_set);
    FD_SET(client_fd, &write_set);

    // 可以利用tv_sec和tv_usec做更小精度的超时控制
    struct timeval tv;
    tv.tv_sec  = 3;
    tv.tv_usec = 0;

    if (select(client_fd + 1, nullptr, &write_set, nullptr, &tv) == 1)
        std::cout << "[select] connect to server successfully." << std::endl;
    else
        std::cout << "[select] connect to server error." << std::endl;

    //5. 关闭socket
    close(client_fd);

    return 0;
}