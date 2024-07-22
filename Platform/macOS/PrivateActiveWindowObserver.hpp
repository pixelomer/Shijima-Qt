#pragma once
#include "../ActiveWindow.hpp"

@class SHJActiveWindowObserver;

namespace Platform {

class PrivateActiveWindowObserver {
private:
    SHJActiveWindowObserver *m_observer;
public:
    PrivateActiveWindowObserver();
    ActiveWindow getActiveWindow();
};

}