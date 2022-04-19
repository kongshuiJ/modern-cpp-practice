//
// Created by kongshui on 22-4-18.
//

#ifndef MODERN_CPP_PRACTICE_REACTOR_SERVER_H
#define MODERN_CPP_PRACTICE_REACTOR_SERVER_H

#include <list>
#include <memory>
#include <thread>
#include <mutex>
#include <array>
#include <condition_variable>

class ReactorServer
{
public:
    typedef std::shared_ptr<std::thread> thread_ptr;

    // 设置单例
    static ReactorServer &getInstance();
    ~ReactorServer();

    ReactorServer(const ReactorServer &server) = delete;
    ReactorServer &operator=(const ReactorServer &server) = delete;

    bool init(const std::string &ip, int16_t port);
    bool stop();

    bool closeClient(int client_fd);

    void mainLoop();

private:

    ReactorServer();

    bool createServerListener(const std::string &ip, int16_t port);

    void acceptThread();
    void workerThread();

private:
    static constexpr int16_t recv_buf_size     = 256;
    static constexpr int8_t  worker_thread_num = 8;
    static constexpr int16_t epoll_event_num   = 1024;

private:
    int     listen_fd_ = 0;
    int     epoll_fd_  = 0;
    bool    init_      = false;
    bool    stop_      = true;
    bool    main_loop_ = true;

    // 一个用于接受新的客户端，一个用于接收客户端发来的数据
    thread_ptr                                accept_thread_;
    std::array<thread_ptr, worker_thread_num> worker_threads_;

    std::condition_variable      accept_cond_;
    std::mutex                   accept_mutex_;

    std::condition_variable      worker_cond_;
    std::mutex                   worker_mutex_;

    // 存在当前可用的客户端
    std::list<int>               clients_list_;
};

#endif //MODERN_CPP_PRACTICE_REACTOR_SERVER_H
