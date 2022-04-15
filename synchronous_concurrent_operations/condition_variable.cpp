//
// Created by kongshui on 2022/4/12.
//
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

class A{
private:
    std::mutex              m_;
    std::condition_variable cv_;
    bool step1_done_ = false;
    bool step2_done_ = false;

public:
    void step1()
    {
        {
            std::lock_guard<std::mutex> l(m_);
            step1_done_ = true;
        }
        std::cout << "1" << std::endl;
        cv_.notify_one();
    }

    void step2()
    {
        std::unique_lock<std::mutex> l(m_);
        cv_.wait(l, [this] { return step1_done_; });
        step2_done_ = true;
        std::cout << "2" << std::endl;
        cv_.notify_one();
    }

    void step3()
    {
        std::unique_lock<std::mutex> l(m_);
        cv_.wait(l, [this] { return step2_done_; });
        std::cout << "3" << std::endl;
    }

private:
    std::mutex              mm_;
    std::condition_variable cvv_;
    bool done_ = false;

public:
    void wait1()
    {
        std::unique_lock<std::mutex> l(mm_);
        cvv_.wait(l, [this] { return done_; });
        std::cout << "11" << std::endl;
    }

    void wait2()
    {
        std::unique_lock<std::mutex> l(mm_);
        cvv_.wait(l, [this] { return done_; });
        std::cout << "22" << std::endl;
    }

    void signal()
    {
        {
            std::lock_guard<std::mutex> l(mm_);
            done_ = true;
        }
        cvv_.notify_all();
    }
};

int main()
{
    A a;
    std::thread t1(&A::step1, &a);
    std::thread t2(&A::step2, &a);
    std::thread t3(&A::step3, &a);

    t1.join();
    t2.join();
    t3.join();

    for (int i = 0; i < 3; ++i)
    {
        std::cout << "index: " << i << std::endl;
        A aa;
        std::thread tt1(&A::wait1, &aa);
        std::thread tt2(&A::wait2, &aa);
        std::thread tt3(&A::signal, &aa);
        tt1.join();
        tt2.join();
        tt3.join();
    }
    return 0;
}