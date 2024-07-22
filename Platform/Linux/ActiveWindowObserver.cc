#include <stdexcept>
#include "../ActiveWindowObserver.hpp"
#include "PrivateActiveWindowObserver.hpp"

namespace Platform {

ActiveWindowObserver::ActiveWindowObserver(): QObject() {
    m_private = new PrivateActiveWindowObserver { this };
    m_private->startKWinScript();
}

int ActiveWindowObserver::tickFrequency() {
    return 1000;
}

void ActiveWindowObserver::tick() {
    if (!m_private->isKWinScriptLoaded()) {
        throw std::runtime_error("KWin script died");
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