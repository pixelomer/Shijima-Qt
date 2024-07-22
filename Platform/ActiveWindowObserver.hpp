#pragma once
#include <QObject>
#include "ActiveWindow.hpp"

namespace Platform {

class PrivateActiveWindowObserver;

class ActiveWindowObserver : public QObject {
private:
    PrivateActiveWindowObserver *m_private = nullptr;
public:
    ActiveWindowObserver();
    int tickFrequency();
    void tick();
    ActiveWindow getActiveWindow();
    ~ActiveWindowObserver();
};

}