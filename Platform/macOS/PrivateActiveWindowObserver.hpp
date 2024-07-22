#pragma once
#include "../ActiveWindow.hpp"

namespace Platform {

class PrivateActiveWindowObserver {
private:
    pid_t m_activePid;
    ActiveWindow m_activeWindow;
public:
    PrivateActiveWindowObserver();
    ActiveWindow getActiveWindow();
};

}