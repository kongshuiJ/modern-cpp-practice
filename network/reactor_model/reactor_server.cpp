//
// Created by kongshui on 22-4-18.
//

#include "reactor_server.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <list>
#include <unistd.h>
#include <cstring>
#include <cerrno>

ReactorServer::ReactorServer()
{
    clients_list_.clear();
    accept_thread_.reset();
    for (auto &thread: worker_threads_)
        thread.reset();

    printf("ReactorServer\n");
}

ReactorServer::~ReactorServer()
{
    stop();
    printf("~ReactorServer\n");
}

ReactorServer &ReactorServer::getInstance()
{
    static ReactorServer server;
    return server;
};

bool ReactorServer::init(const std::string &ip, int16_t port)
{
    if (init_)
    {
        printf("already init successed");
        return true;
    }

    if (!createServerListener(ip, port))
    {
        printf("create server listener failed\n");
        return false;
    }

    accept_thread_.reset(new std::thread(&ReactorServer::acceptThread, this));

    for (auto &thread: worker_threads_)
        thread.reset(new std::thread(&ReactorServer::workerThread, this));

    init_ = true;
    printf("init success\n");

    return init_;
}

bool ReactorServer::stop()
{
    if (stop_)
    {
        printf("already stop successed");
        return true;
    }

    main_loop_ = false;
    accept_cond_.notify_all();
    worker_cond_.notify_all();
    printf("notify all thread(accept, worker)\n");

    accept_thread_->join();
    for (auto &thread: worker_threads_)
        thread->join();
    printf("join all success(accept, worker)\n");

    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, listen_fd_, nullptr);
    shutdown(listen_fd_, SHUT_RDWR);
    close(listen_fd_);
    close(epoll_fd_);
    stop_ = true;

    return stop_;
}

bool ReactorServer::closeClient(int client_fd)
{
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr) == -1)
        printf("close client socket failed as call epoll_ctl failed\n");

    close(client_fd);

    return true;
}

void ReactorServer::mainLoop()
{
    while (main_loop_)
    {
        struct epoll_event ev[epoll_event_num] = {0};
        int result = epoll_wait(epoll_fd_, ev, epoll_event_num, 10);
        if (result == 0)
            continue;
        else if (result < 0)
        {
            printf("epoll_wait error\n");
            continue;
        }

        int num = result > epoll_event_num ? epoll_event_num : result;
        for (int idx = 0; idx < num; ++idx)
        {
            if (ev[idx].data.fd == listen_fd_)
                accept_cond_.notify_one();
            else
            {
                {
                    std::unique_lock<std::mutex> guard(worker_mutex_);
                    clients_list_.push_back(ev[idx].data.fd);
                }

                worker_cond_.notify_one();
            }
        }
    }

    printf("main loop exit\n");
}

void ReactorServer::acceptThread()
{
    while (true)
    {
        int new_fd         = -1;
        socklen_t addr_len = 0;
        struct sockaddr_in client_addr{};
        {
            std::unique_lock<std::mutex> guard(accept_mutex_);
            accept_cond_.wait(guard);

            if (!main_loop_)
                break;

            new_fd = accept(listen_fd_, (struct sockaddr *) &client_addr, &addr_len);
        }

        if (new_fd == -1)
            continue;

        printf("new client connected: %s：%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        int old_flag = fcntl(new_fd, F_GETFL, 0);
        int new_flag = old_flag | O_NONBLOCK;
        if (fcntl(new_fd, F_SETFL, new_flag) == -1)
        {
            printf("fcntl error, old_flag = %d, new_flag = %d\n", old_flag, new_flag);
            continue;
        }

        struct epoll_event ev{};
        ev.events  = EPOLLIN | EPOLLRDHUP | EPOLLET;
        ev.data.fd = new_fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1)
            printf("epoll_ctl error, fd = %d\n", ev.data.fd);
    }

    printf("accept thread exit\n");
}

void ReactorServer::workerThread()
{
    while (true)
    {
        int client_fd = -1;
        {
            std::unique_lock<std::mutex> gurad(worker_mutex_);
            while (clients_list_.empty())
            {
                if (!main_loop_)
                {
                    printf("worker thread exit\n");
                    return;
                }

                worker_cond_.wait(gurad);
            }

            client_fd = clients_list_.front();
            clients_list_.pop_front();
        }

        std::string client_msg;
        bool error              = false;
        char buf[recv_buf_size] = {0};

        while (true)
        {
            memset(buf, 0, sizeof(buf));
            int result = recv(client_fd, buf, recv_buf_size, 0);
            if (result == -1)
            {
                if (errno == EWOULDBLOCK)
                    break;
                else
                {
                    printf("recv error, client disconnected, fd = %d\n", client_fd);
                    closeClient(client_fd);
                    error = true;
                    break;
                }
            } else if (result == 0)
            {
                printf("peer closed, client disconnected, fd = %d\n", client_fd);
                closeClient(client_fd);
                error = true;
                break;
            }

            client_msg += buf;
        }

        if (error)
            continue;

        printf("client msg: %s\n", client_msg.c_str());

        client_msg += " test send";

        int result = send(client_fd, client_msg.c_str(), client_msg.length(), 0);
        if (result == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else
            {
                printf("send error, fd = %d\n", client_fd);
                closeClient(client_fd);
                break;
            }

        }
    }
}

bool ReactorServer::createServerListener(const std::string &ip, int16_t port)
{
    listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listen_fd_ == -1)
        return false;

    int on = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEPORT, (char *) &on, sizeof(on));

    struct sockaddr_in server_addr{};
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    server_addr.sin_port        = htons(port);

    if (bind(listen_fd_, (sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
        printf("bind failed\n");
        return false;
    }

    if (listen(listen_fd_, 50) == -1)
    {
        printf("listen failed\n");
        return false;
    }

    epoll_fd_ = epoll_create(1);
    if (epoll_fd_ == -1)
    {
        printf("epoll_create failed\n");
        return false;
    }

    struct epoll_event ev{};
    ev.events  = EPOLLIN | EPOLLRDHUP;
    ev.data.fd = listen_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &ev) == -1)
        return false;

    return true;
}