#pragma once
#include <cstdbool>
#include <QString>

namespace Platform {

class ActiveWindow {
public:
    bool available;
    QString uid;
    long pid;
    double x, y, width, height;
    ActiveWindow(QString const& uid, long pid, double x, double y,
        double width, double height):
        available(true), uid(uid), pid(pid), x(x), y(y), width(width),
        height(height) {}
    ActiveWindow(): available(false) {}
};

}