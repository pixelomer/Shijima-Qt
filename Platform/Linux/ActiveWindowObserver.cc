#include <stdexcept>
#include "../ActiveWindowObserver.hpp"
#include "PrivateActiveWindowObserver.hpp"

namespace Platform {

ActiveWindowObserver::ActiveWindowObserver(): QObject() {
    m_private = new PrivateActiveWindowObserver { this };
}

int ActiveWindowObserver::tickFrequency() {
    return 1000;
}

void ActiveWindowObserver::tick() {
    if (!m_private->alive()) {
        throw std::runtime_error("Active window observer died");
    }
}

ActiveWindow ActiveWindowObserver::getActiveWindow() {
    return m_private->getActiveWindow();
}

ActiveWindowObserver::~ActiveWindowObserver() {
    delete m_private;
    m_private = nullptr;
}

}