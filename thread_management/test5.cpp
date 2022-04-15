//
// Created by kongshui on 2022/4/12.
//

// std::thread 是 move-only 类型，不能拷贝，
// 只能通过移动转移所有权，但不能转移所有权到 joinable 的线程
#include <iostream>
#include <thread>
#include <utility>
#include <cassert>

void f() {}

void g() {}

int main()
{
    std::thread a{f};
    std::thread b = std::move(a);
    assert(!a.joinable());
    assert(b.joinable());
    a = std::thread{g};
    assert(a.joinable());
    assert(b.joinable());
    // a = std::move(b);  // 错误，不能转移所有权到 joinable 的线程
    a.join();
    a = std::move(b);
    assert(a.joinable());
    assert(!b.joinable());
    a.join();
}