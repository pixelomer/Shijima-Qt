#pragma once
#include <cstdbool>

namespace Platform {

class ActiveWindow {
public:
    long pid;
    bool available;
    double x, y, width, height;
    ActiveWindow(long pid, double x, double y, double width, double height):
        available(true), x(x), y(y), width(width), height(height) {}
    ActiveWindow(): available(false) {}
};

}