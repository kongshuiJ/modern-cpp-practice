//
// Created by kongshui on 2022/4/12.
//

#include <iostream>
#include <thread>
#include <cassert>

void f(int &i) { ++i; }

int main()
{
    int i = 1;
    std::thread t{f, std::ref(i)};
    t.join();
    assert(i == 2);

    return 0;
}
