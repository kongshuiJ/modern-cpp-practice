//
// Created by kongshui on 2022/4/12.
//

#include <iostream>
#include <thread>
#include <utility>

class A
{
public:
    void f(int i) { std::cout << i << std::endl; }
};

void f(std::unique_ptr<int> p) { std::cout << *p << std::endl; }

int main()
{
    A a;
    std::thread t1{&A::f, &a, 42};  // 调用 a->f(42)
    std::thread t2{&A::f, a, 42};   // 拷贝构造 tmp_a，再调用 tmp_a.f(42)
    t1.join();
    t2.join();

    std::unique_ptr<int> p(new int(42));
    std::thread          t{f, std::move(p)};
//    *p = 3;
//    std::cout << "pppp:" << *p << std::endl;
    t.join();

    return 0;
}