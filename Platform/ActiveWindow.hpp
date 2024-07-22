#pragma once
#include <cstdbool>

namespace Platform {

class ActiveWindow {
public:
    bool available;
    double x, y, width, height;
    ActiveWindow(double x, double y, double width, double height):
        x(x), y(y), width(width), height(height), 
        available(true) {}
    ActiveWindow(): available(false) {}
};

}