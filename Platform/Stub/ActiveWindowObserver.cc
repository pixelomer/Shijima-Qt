#include <stdexcept>
#include "../ActiveWindowObserver.hpp"

namespace Platform {

ActiveWindowObserver::ActiveWindowObserver(): QObject() {
}

int ActiveWindowObserver::tickFrequency() {
    return 0;
}

void ActiveWindowObserver::tick() {
}

ActiveWindow ActiveWindowObserver::getActiveWindow() {
    return {};
}

ActiveWindowObserver::~ActiveWindowObserver() {
}

}