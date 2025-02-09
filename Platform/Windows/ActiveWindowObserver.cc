#include <stdexcept>
#include "../ActiveWindowObserver.hpp"
#include "PrivateActiveWindowObserver.hpp"
#include <iostream>

namespace Platform {

ActiveWindowObserver::ActiveWindowObserver(): QObject() {
    m_private = new PrivateActiveWindowObserver;
}

int ActiveWindowObserver::tickFrequency() {
    return 0;
}

void ActiveWindowObserver::tick() {
}

ActiveWindow ActiveWindowObserver::getActiveWindow() {
    return m_private->getActiveWindow();
}

ActiveWindowObserver::~ActiveWindowObserver() {
    delete m_private;
}

}