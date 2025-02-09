#pragma once
#include "../ActiveWindow.hpp"

namespace Platform {

class PrivateActiveWindowObserver {
private:
    double m_scaleRatio;
    ActiveWindow m_activeWindow;
public:
    PrivateActiveWindowObserver();
    ActiveWindow getActiveWindow();
};

}