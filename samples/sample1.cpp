#include <iostream>

class Point
{
public:

    double x;
    double y;

    Point(double xx, double yy) : x(xx), y(yy) {}
};

int main()
{
    Point myPoint(10.0, 20.0);
    auto[myX, myY] = myPoint;

    std::cout << myX << std::endl;
    std::cout << myY << std::endl;
}