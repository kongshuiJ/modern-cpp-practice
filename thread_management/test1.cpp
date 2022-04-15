//
// Created by kongshui on 2022/4/12.
//

#include <iostream>
#include <thread>

void f()
{
    std::cout << "hello world" << std::endl;
}

int main()
{
    std::thread t{f};
    t.join();

    return 0;
}